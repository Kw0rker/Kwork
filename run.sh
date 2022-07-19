FILE_EXTENSION=".kwac"
build/kwork_compiler $1 >>/dev/null
echo "compilation complete"
build/kwork_assembler input.kwac >>/dev/null
echo "assembling complete"
echo ""
build/kwork_kernel 0
