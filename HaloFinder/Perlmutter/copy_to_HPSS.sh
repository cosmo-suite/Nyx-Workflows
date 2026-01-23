src="/pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/Frontier_Output_8192/Output_8192/LightCones/SimpleBinary"
dst="/home/projects/nyx/nataraj2/Output_8192/LightCones/SimpleBinary"

for file in $src/*.bin; do
    base=$(basename "$file")
    hsi "put $file : $dst/$base"
done

src="/pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/Frontier_Output_8192/Output_8192/Halos/SimpleBinary"
dst="/home/projects/nyx/nataraj2/Output_8192/Halos/SimpleBinary"

for file in $src/*.bin; do
    base=$(basename "$file")
    hsi "put $file : $dst/$base"
done
