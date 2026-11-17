#!/bin/bash
# For each plotfile listed in plt_filelist.txt (one path per line, '#' comments
# and blank lines ignored):
#   1. convert the plotfile to HDF5 with convert3d
#   2. run p1d_all_axes.ex on it
#   3. append "z tau_eff" to a single summary file (tau_eff_vs_z.txt)
#
# Run inside an allocation (see run_interactive_CPU.sh).
# Usage: sh run_compute_lyap_1d_and_tau_eff.sh --reionization-data-file=<file> [plt_filelist.txt]
#
# A failure on one plotfile is reported and that plotfile is skipped.

# Re-exec under bash if started via plain sh (script uses bash features).
if [ -z "$BASH_VERSION" ]; then exec bash "$0" "$@"; fi

REION=
LIST=
for arg in "$@"; do
    case "$arg" in
        --reionization-data-file=*) REION="${arg#*=}" ;;
        *) LIST="$arg" ;;
    esac
done
LIST=${LIST:-plt_filelist.txt}

if [ -z "$REION" ]; then
    echo "Usage: sh $0 --reionization-data-file=<file> [plt_filelist.txt]" >&2
    exit 1
fi
if [ ! -f "$REION" ]; then
    echo "Reionization data file $REION not found" >&2
    exit 1
fi

NTASKS=512
CONVERT=./convert3d.gnu.x86-milan.PROF.MPI.ex
P1D=p1d_all_axes.ex
H5DIR=h5_snapshots
OUTDIR=p1d_output
SUMMARY=tau_eff_vs_z.txt

if [ ! -f "$LIST" ]; then
    echo "File list $LIST not found" >&2
    exit 1
fi

mkdir -p "$H5DIR" "$OUTDIR"

# Fresh summary file with header.
echo "# z tau_eff=-ln<exp(-tau)> (unscaled tau, mean over x,y,z axes)" > "$SUMMARY"

while IFS= read -r plt || [ -n "$plt" ]; do
    # skip blank lines and comments
    [[ -z "${plt// }" || "$plt" =~ ^[[:space:]]*# ]] && continue

    name=$(basename "$plt")
    h5="$H5DIR/${name}.h5"
    prefix="$OUTDIR/p1d_${name}_"

    echo "=== $plt"

    # 1. plotfile -> HDF5. Skip only if an existing file is complete (has
    #    baryon_density); a truncated file from an interrupted conversion is redone.
    if [ -f "$h5" ] && h5ls "$h5/native_fields/baryon_density" >/dev/null 2>&1; then
        echo "  $h5 exists and looks complete, skipping conversion"
    else
        [ -f "$h5" ] && echo "  $h5 is incomplete, reconverting"
        # Convert to a temp name and rename on success so a failed/interrupted
        # run never leaves a partial file under the final name.
        tmp="${h5}.partial"
        rm -f "$h5" "$tmp"
        srun --input=none -n $NTASKS $CONVERT input_path="$plt" output_path="$tmp" || {
            echo "  conversion failed for $plt, skipping" >&2
            rm -f "$tmp"
            continue
        }
        mv "$tmp" "$h5"
    fi

    # 2. P1D / optical depth. p1d_all_axes appends to <prefix>tau_eff.txt, so
    #    remove any stale copy first.
    rm -f "${prefix}tau_eff.txt"
    srun --input=none -n $NTASKS $P1D "$h5" --reionization-data-file="$REION" "$prefix" || {
        echo "  p1d_all_axes.ex failed for $h5, skipping" >&2
        continue
    }

    # 3. Append this snapshot's (z, tau_eff) line to the summary.
    if [ -f "${prefix}tau_eff.txt" ]; then
        grep -v '^#' "${prefix}tau_eff.txt" >> "$SUMMARY"
    else
        echo "  no tau_eff output found for $name" >&2
    fi
done < "$LIST"

echo "Done. Summary in $SUMMARY"
