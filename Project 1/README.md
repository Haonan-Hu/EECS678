# eecs-678-quash
# Made by Chance Penner and Haonan Hu
# TESTING REQUIREMENTS
Ensure that the testing files are made properly. DO NOT copy paste test files. You MUST create a file within vscode and then copy the contents over. If you use a pre-made text file, it will NOT work. Make sure there is a trailing newline at the end as well. If you do not us vscode to create the test file, the input will be weird. This is because many different editors use different styles of saving whitespace, newlines, etc., so please just use vscode to create a new file and then you may copy the text into that file and add a newline at the end.

To summarize, basically when using textEdit and Atom and vscode, we were getting mixed results with reading from files. We found the best solution was to stick with just vscode. For example, when using the premade ‘command6.txt’ and ‘command7.txt’ files, they parsed weirdly, either failing at the end or not reading the last command in the file.
Instead, just open vscode and create a new file there. Then, you can copy over the contents of a text file into the newly created one and save it. We found that having a newline at the end made everything work better. For example:
uname

Notice how there is a newline at the end! This works best. We have sample test files in the directory, labeled ‘test1.txt’ through ‘test13.txt’ (note, these are copies of the given tests 1,2,3,5, alongside modified command6.txt and command7.txt. And then some we made).
Some of the copies of tests 1-7 had to be modified to work on our machine. For example, the original command7.txt was just:
file7
set MYVAR="NEW_ENV_VAR"
file7
quit
But this only works if you first have file7 in your path, so we changed it to:
set PATH=/mnt/c/Users/Chance/Desktop/eecs-678-quash
file7
set MYVAR="NEW_ENV_VAR"
file7
quit

Again, note the newline at the end! This way, file7 was in the PATH and would run as intended. Another way to do this would be to just add ‘.’ to the PATH and then cd to that directory, or cd to that directory and change it to use ./file7 instead. We just chose the way we thought was best.

# Build Instructions
Just type 'make' to build.
Then run with './quash'
To run test files type './quash < testFile.txt'