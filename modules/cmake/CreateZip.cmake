# create_zip(<output_file> <input_files> <working_dir>)
#
# Creates a zip file for the given input files (at configure time, not build time).
function(create_zip output_file input_files working_dir)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar "cf" "${output_file}" --format=zip -- ${input_files}
        WORKING_DIRECTORY "${working_dir}"
        OUTPUT_FILE  "${output_file}"
        INPUT_FILE ${input_files}
    )
endfunction()