#!/bin/bash

function read_libs()
{
    string=$(${host}-otool -L $1 \
                    | perl -pe 's/.*:\n//g' \
                    | perl -pe 's/\t([^\t]+) \(compatibility version ([0-9]+.[0-9]+.[0-9]+), current version ([0-9]+.[0-9]+.[0-9]+)\)\n/\1;/g')    
    
    IFS=';' read -r -a libs <<< "${string}"
}

origin="/build/speech-analysis"
targets=("${origin}")
real_targets=("${origin}")
processed=()

dd if=/dev/zero of=/dist/speech-analysis.dmg bs=1M count=10
mkfs.hfsplus -v "Speech analysis" /dist/speech-analysis.dmg
mkdir -p /dmg
mount -o loop /dist/speech-analysis.dmg /dmg

mkdir -p /dmg/SpeechAnalysis.app/Contents/MacOS
cp -v /src/dist-res/Info.plist /dmg/SpeechAnalysis.app/Contents
cp -v /src/Montserrat.otf /dmg/SpeechAnalysis.app/Contents/MacOS

while [ ${#targets[@]} -gt 0 ]; do
    current=${targets[0]}
    real_current=${real_targets[0]}

    targets=("${targets[@]:1}")
    real_targets=("${real_targets[@]:1}")

    if [[ " ${processed[@]} " =~ " ${current} " ]]; then
        continue
    fi

    read_libs $real_current

    cp -v "$real_current" /dmg/SpeechAnalysis.app/Contents/MacOS

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
    
            lib_name=$(basename "${lib}")
            ${host}-install_name_tool -change "${lib}" "@executable_path/${lib_name}" /dmg/SpeechAnalysis.app/Contents/MacOS/${current_name}
        fi
    done

    ${host}-install_name_tool -id "@executable_path/${current_name}" /dmg/SpeechAnalysis.app/Contents/MacOS/${current_name}

    targets+=("${filtered_libs[@]}")
    real_targets+=("${filtered_real_libs[@]}")
    processed+=("${current}")
done

umount /dmg
