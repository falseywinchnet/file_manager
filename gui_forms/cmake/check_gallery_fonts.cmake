if(NOT IS_DIRECTORY "${FONT_DIRECTORY}")
    message(FATAL_ERROR "Bundled font directory is missing: ${FONT_DIRECTORY}")
endif()

set(font_manifest
    "PortsmouthRapids.ttf|b7a98b9dc091f7319a658b1e924ecd97948ec4a1eea0a772501e893107d04f19"
    "PortsmouthRapids-Bold.ttf|e9dd60dcd8198235fe531149e0b1ab282e28655cc2159ead6123ee398deb1f82"
    "Carlito-Regular.ttf|f6418f708baede9789daef5d458c0f53d2a888af9820e8062934e504fedc6595"
    "Carlito-Bold.ttf|bb5d20f79b82599ec72983597437373a80f2d2085fa91fc144fd74e876a594db"
    "Carlito-Italic.ttf|0b019225e58d702bfedcbd35c21696769f8ee115cb6343f84c2f240312450d1c"
    "Carlito-BoldItalic.ttf|b32928186c119599e03ca6a1ffc680fdcb7fac95772f4b95d989cf6cd3861517"
    "Cousine-Regular.ttf|5a57f0184000371cb22fe3fcea4c500354cab1a69efaf7beaf3e6eca6ecfefea"
    "Cousine-Bold.ttf|331215ec6445f41e98d8971251bb6237e722907f4101faae990583accfe79545"
    "Cousine-Italic.ttf|afb869eee8643d09915c3286edb61dad66278ceb1511c79b474ce21548421bcd"
    "Cousine-BoldItalic.ttf|7f804549c941ceaac0a9635389ed09c4bbc86e4bfe37589191dacd1b5b0e26a5"
    "NotoSansCJKjp-Regular.otf|68a3fc98800b2a27b371f2fb79991daf3633bd89309d4ffaa6946fd587f375b5"
    "NotoEmoji-Regular.ttf|415dc6290378574135b64c808dc640c1df7531973290c4970c51fdeb849cb0c5"
    "OFL-NotoSansCJKjp.txt|6a73f9541c2de74158c0e7cf6b0a58ef774f5a780bf191f2d7ec9cc53efe2bf2"
    "OFL-NotoEmoji.txt|6a73f9541c2de74158c0e7cf6b0a58ef774f5a780bf191f2d7ec9cc53efe2bf2")

foreach(entry IN LISTS font_manifest)
    string(REPLACE "|" ";" fields "${entry}")
    list(GET fields 0 filename)
    list(GET fields 1 expected_hash)
    set(path "${FONT_DIRECTORY}/${filename}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Bundled font face is missing: ${filename}")
    endif()
    file(SHA256 "${path}" actual_hash)
    if(NOT actual_hash STREQUAL expected_hash)
        message(FATAL_ERROR
            "Bundled font hash changed for ${filename}: ${actual_hash}")
    endif()
endforeach()
