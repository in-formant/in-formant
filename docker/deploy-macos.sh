#!/bin/bash

function read_libs()
{
    string=$(${host}-otool -L $1 \
                    | perl -pe 's/.*:\n//g' \
                    | perl -pe 's/\t([^\t]+) \(compatibility version ([0-9]+.[0-9]+.[0-9]+), current version ([0-9]+.[0-9]+.[0-9]+)\)\n/\1;/g')    
    
    IFS=';' read -r -a libs <<< "${string}"
}

origin="/build/pitch-tracker"
targets=("${origin}")
real_targets=("${origin}")
processed=()

dd if=/dev/zero of=/dist/pitch-tracker.dmg bs=1M count=7
mkfs.hfsplus -v "Pitch tracker" /dist/pitch-tracker.dmg
mkdir -p /dmg
mount -o loop /dist/pitch-tracker.dmg /dmg

mkdir -p /dmg/PitchTracker.app/Contents/MacOS
cp -v /src/dist-res/Info.plist /dmg/PitchTracker.app/Contents

while [ ${#targets[@]} -gt 0 ]; do
    current=${targets[0]}
    real_current=${real_targets[0]}

    targets=("${targets[@]:1}")
    real_targets=("${real_targets[@]:1}")

    if [[ " ${processed[@]} " =~ " ${current} " ]]; then
        continue
    fi

    read_libs $real_current

    cp -v "$real_current" /dmg/PitchTracker.app/Contents/MacOS

    current_name=$(basename "${current}")
    
    filtered_libs=()
    filtered_real_libs=()

    for lib in "${libs[@]}"; do
        actual_file=$(echo "${lib}" \
                        | sed -e 's/\/opt\/local\/\(.*\)/\/osxcross\/target\/macports\/pkgs\/opt\/local\/\1/g' \
                        | sed -e 's/\/\(usr\|System\)\/\(.*\)//g' \
                        | sed -e 's/@rpath\(.*\)//g')

        if [ -n "${actual_file}" ]; then
            filtered_libs+=("${lib}")
            filtered_real_libs+=("${actual_file}")
        fi
         
        lib_name=$(basename "${lib}")
        ${host}-install_name_tool -change "${lib}" "@rpath/${lib_name}" /dmg/PitchTracker.app/Contents/MacOS/${current_name}
    done

    ${host}-install_name_tool -id "@rpath/${current_name}" /dmg/PitchTracker.app/Contents/MacOS/${current_name}

    targets+=("${filtered_libs[@]}")
    real_targets+=("${filtered_real_libs[@]}")
    processed+=("${current}")
done

umount /dmg
