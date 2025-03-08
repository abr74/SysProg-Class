#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Basic command execution: echo" {
    run ./dsh <<EOF
echo "Testing dsh shell"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "Testing dsh shell" ]]
}

@test "Built-in command: changing directories and listing files" {
    run ./dsh <<EOF
cd /
ls
EOF
    
    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
    [[ "$output" =~ "bin" ]] || [[ "$output" =~ "usr" ]]  
}

@test "Pipeline: find | wc" {
    run ./dsh <<EOF
find . -type f | wc -l
EOF
    
    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
    [[ "$output" =~ [0-9]+ ]] 
}



@test "Redirection: Save output to a file" {
    run ./dsh <<EOF
echo "File Output Test" > output.txt
cat output.txt
EOF
    
    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
    [[ "$output" =~ "File Output Test" ]]
}


@test "Redirection with appending" {
    run ./dsh <<EOF
echo "First line" > append_test.txt
echo "Second line" >> append_test.txt
cat append_test.txt
EOF

    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
    [[ "$output" =~ "First line" ]]
    [[ "$output" =~ "Second line" ]]
}

@test "Background execution with &" {
    run ./dsh <<EOF
sleep 2 &
echo "Process running"
EOF

    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
    [[ "$output" =~ "Process running" ]]
}

@test "Exit command should terminate session" {
    run ./dsh <<EOF
echo "Before exit"
exit
echo "After exit"
EOF

    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
    [[ ! "$output" =~ "After exit" ]]  
}