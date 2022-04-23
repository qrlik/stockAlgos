NEW_TOOLCHAIN=false

while [ "$1" != "" ]; do
	case "$1" in
		"-nt")
			NEW_TOOLCHAIN=true
		;;
	esac
    shift
done

echo -- building WIN64 project

mkdir -p ../../build/
cd ../../build/

if [ $NEW_TOOLCHAIN == true ]; then
	cmake ../ -G \
		"Visual Studio 17 2022" \
		-A \
		Win32 \
		-T \
		v143
else
	cmake ../ -G \
		"Visual Studio 16 2019" \
		-A \
		Win32 \
		-T \
		v142
fi
