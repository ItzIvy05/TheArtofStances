vcpkg_from_github(OUT_SOURCE_PATH SOURCE_PATH REPO Vermunds/ModConfigUI REF d88a7f2bd6aa6bf9789018008b6b5854f24bcfae SHA512 2281beec7d3a85907710f908fdf7ee1121f00d3ddeb850987d7447004eb46ad97bb4ecb6b21b9f4d89738b774c5af6322167ce09de43d3205ec2dafc610b7510 HEAD_REF master)

# commonlibsse-ng in place of po3's commonlib
vcpkg_replace_string("${SOURCE_PATH}/include/PCH.h" "namespace logger\n{\n\ttemplate <class... T>\n\tusing trace = REX::TRACE<T...>;\n\ttemplate <class... T>\n\tusing debug = REX::DEBUG<T...>;\n\ttemplate <class... T>\n\tusing info = REX::INFO<T...>;\n\ttemplate <class... T>\n\tusing warn = REX::WARN<T...>;\n\ttemplate <class... T>\n\tusing error = REX::ERROR<T...>;\n\ttemplate <class... T>\n\tusing critical = REX::CRITICAL<T...>;\n}" "namespace logger = SKSE::log;")
vcpkg_replace_string("${SOURCE_PATH}/src/ModConfigUI/Localization.cpp" "if (!REX::UTF16_TO_UTF8(wide, a_contents))" "std::optional<std::string> converted = SKSE::stl::utf16_to_utf8(wide);\n\t\tif (!converted)")
vcpkg_replace_string("${SOURCE_PATH}/src/ModConfigUI/Localization.cpp" "\t\treturn true;\n\t}\n\n\tbool ParseFile" "\t\ta_contents = std::move(*converted);\n\t\treturn true;\n\t}\n\n\tbool ParseFile")
vcpkg_replace_string("${SOURCE_PATH}/src/ModConfigUI/FUCKBackend.cpp" "// FUCK_API.h still logs through SKSE::log, which CommonLib no longer provides\nnamespace SKSE::log\n{\n\tusing spdlog::error;\n\tusing spdlog::info;\n}\n\n" "")
vcpkg_replace_string("${SOURCE_PATH}/src/ModConfigUI/SKSEMenuFrameworkBackend.cpp" "REL::ID{ 68617 }" "RELOCATION_ID(67315, 68617)")
vcpkg_replace_string("${SOURCE_PATH}/src/ModConfigUI/SKSEMenuFrameworkBackend.cpp" "static REL::Trampoline trampoline" "static SKSE::Trampoline trampoline")

file(INSTALL "${SOURCE_PATH}/include/ModConfigUI" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${SOURCE_PATH}/include/PCH.h" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
file(INSTALL "${SOURCE_PATH}/src/ModConfigUI/" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}/src")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE" "${SOURCE_PATH}/EXCEPTIONS.md")
