set(MACRO_sourceFileExtensions "*.h" "*.hpp" "*.c" "*.cpp")

#relative path + sources list
macro(GENERATE_SOURCE_GROUPS RELATIVE_PATH)
	string(LENGTH ${RELATIVE_PATH} macroSrcPathLength)
	foreach(macroSourceFilePath ${ARGN})
		if (${macroSourceFilePath} MATCHES ${RELATIVE_PATH})
			string(SUBSTRING ${macroSourceFilePath} ${macroSrcPathLength} -1 macroRelativeSourcePath)
			string(FIND ${macroRelativeSourcePath} / macroPos REVERSE)
			string(SUBSTRING ${macroRelativeSourcePath} 0 ${macroPos} macroSourceGroupName)
			string(REPLACE / \\\\ macroSourceGroupName ${macroSourceGroupName})
			source_group(${macroSourceGroupName} FILES ${macroSourceFilePath})
		endif()
	endforeach(macroSourceFilePath)
	unset(macroSrcPathLength)
	unset(macroSourceGroupName)
	unset(macroRelativeSourcePath)
	unset(macroPos)
endmacro()