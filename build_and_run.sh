clear

clean="clean"
root_build="build"

if [[ $1 == clean ]]; then
    echo "Removing folder: $root_build"
    rm -rf build

    if [[ -d $root_build ]]; then
        echo "FAILED TO REMOVE $root_build folder !!!!"
    fi
fi

if [[ ! -d $root_build ]]; then
    mkdir build
fi

if [[ -L "compile_commands.json" ]]; then
    echo "Removed compile_commands link"
    rm compile_commands.json
fi

cd build || exit
cmake -DCMAKE_BUILD_TYPE=Debug .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make
ln -s compile_commands.json ../

./game_run

cd ..
