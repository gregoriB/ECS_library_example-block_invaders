clear

root_build="build"
libs_folder="libraries/"
ecs_source="$HOME/coding/cpp/ecs"
ecs_folder="libraries/ecs"
library_build="libraries/build"
game_build="game/build"

if [[ $1 == "update" || ! -d $ecs_folder ]]; then
    echo "Reinstalling ECS library"
    if [[ -d $ecs_folder ]]; then
        rm -rf $ecs_folder
    fi

    cp -r $ecs_source $libs_folder
fi


if [[ -d $root_build ]]; then
    echo "Removing folder: $root_build"
    rm -rf build

    if [[ -d $root_build ]]; then
        echo "FAILED TO REMOVE $root_build folder !!!!"
    fi
fi

if [[ -d $root_build ]]; then
    echo "Removing folder: $root_build"
    rm -rf build

    if [[ -d $root_build ]]; then
        echo "FAILED TO REMOVE $root_build folder !!!!"
    fi
fi

if [[ -d $library_build ]]; then
    echo "Removing folder: $library_build"
    rm -rf library_build/build

    if [[ -d $library_build ]]; then
        echo "FAILED TO REMOVE $library_build !!!!"
    fi
fi

if [[ -d $game_build ]]; then
    echo "Removing folder: $game_build"
    rm -rf game_build/build
    if [[ -d $game_build ]]; then
        echo "FAILED TO REMOVE $game_build !!!!"
    fi
fi

if [[ -L "compile_commands.json" ]]; then
    echo "Removed compile_commands link"
    rm compile_commands.json
fi

mkdir build

cd build || exit

rm -f CMakeCache.txt

cmake --debug-output .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

make VERBOSE=1

ln -s compile_commands.json ../

cd ..
