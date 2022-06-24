./kwork_compiler $1 >>/dev/null
echo "compilation complete"
./kwork_assembler input.kwac >>/dev/null
echo "assembling complete"
echo ""
./kwork_kernel