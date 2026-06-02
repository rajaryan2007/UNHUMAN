#!/bin/bash
echo "Running Setup Script..."
python3 script/Setup.py
if [ $? -ne 0 ]; then
  echo "Setup failed!"
  exit 1
fi

echo ""
echo "Generating Ninja Build Files..."
cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [ -f "build/compile_commands.json" ]; then
  ln -sf build/compile_commands.json compile_commands.json
  echo "Symlinked compile_commands.json to root."
fi

echo ""
echo "Project generation complete! Run 'cmake --build build' to compile."
