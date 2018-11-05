# Steca Windows packaging #

set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-Installer")

set(CPACK_GENERATOR IFW)

set(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY "") # empty => default is install to top-level (?)
set(CPACK_IFW_TARGET_DIRECTORY "@HomeDir@/${CMAKE_PROJECT_NAME}")
set(CPACK_IFW_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/main/icons/Steca.ico")
set(CPACK_IFW_PACKAGE_WINDOW_ICON "${CMAKE_SOURCE_DIR}/main/icons/Steca.png")

set(CPACK_COMPONENTS_GROUPING IGNORE)
set(CPACK_IFW_PACKAGE_GROUP Steca)

include(CPack)
include(CPackIFW)

#The following code was adapted from "Using windeployqt with CPack":
#https://hk.saowen.com/a/d1cf90fcfea6d511629fd5a6c8113808721a7f19656677e8a5fab370a8d35cd4
#########################################################################################

# Add some required libraries
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

#Find windeployqt
find_package(Qt5Core REQUIRED)
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

# Add commands that copy the Qt runtime to the target's output directory after
# build and install the Qt runtime to the specified directory
function(windeployqt target directory)

  # Run windeployqt immediately after build
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
    env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
    --verbose 0
    --no-compiler-runtime
    \"$<TARGET_FILE:${target}>\"
    )

  # install(CODE ...) doesn't support generator expressions, but
  # file(GENERATE ...) does - store the path in a file
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}_$<CONFIG>"
    CONTENT "$<TARGET_FILE:${target}>"
    CONDITION "$<CONFIG:Release>"
    )

  # Before installation, run a series of commands that copy each of the Qt
  # runtime files to the appropriate directory for installation
  install(CODE
    "
    file(READ \"${CMAKE_CURRENT_BINARY_DIR}/${target}_Release\" _file)
    execute_process(
      COMMAND \"${CMAKE_COMMAND}\" -E
      env PATH=\"${_qt_bin_dir}\" \"${WINDEPLOYQT_EXECUTABLE}\"
      --dry-run
      --no-compiler-runtime
      --list mapping
      \${_file}
      OUTPUT_VARIABLE _output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    separate_arguments(_files WINDOWS_COMMAND \${_output})
    while(_files)
      list(GET _files 0 _src)
      list(GET _files 1 _dest)
      execute_process(
        COMMAND \"${CMAKE_COMMAND}\" -E
        copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/${directory}/\${_dest}\"
        )
      list(REMOVE_AT _files 0 1)
    endwhile()
    "
    COMPONENT "QtLibs"
    )
  # windeployqt doesn't work correctly with the system runtime libraries,
  # so we fall back to one of CMake's own modules for copying them over
  foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
    get_filename_component(filename "${lib}" NAME)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E
      copy_if_different "${lib}" \"$<TARGET_FILE_DIR:${target}>\"
      )
  endforeach()
endfunction()

mark_as_advanced(WINDEPLOYQT_EXECUTABLE)
#########################################################################################

# 1. add/configure component group
# 2. install component
# 3. add/configure component
cpack_add_component_group(Steca EXPANDED)
cpack_ifw_configure_component_Group(Steca NAME fzj.jcns.scg.steca)
install(TARGETS Steca
  DESTINATION bin
  COMPONENT Steca.exe)
windeployqt("Steca" "bin")
cpack_add_component(Steca.exe ENABLED GROUP Steca)
cpack_ifw_configure_component(Steca.exe NAME fzj.jcns.scg.steca.exe SCRIPT ${CMAKE_SOURCE_DIR}/StecaInstallScript.qs)
