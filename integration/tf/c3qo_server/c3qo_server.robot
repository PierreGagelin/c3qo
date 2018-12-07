*** Settings ***
Library    Process

*** Variables ***
${VERSION}    0.0.7    # Just here to keep "Variables"

*** Test Cases ***
Check Server Startup
    [Documentation]    Various ways to start c3qo server
    Run Server
    Run Server With Help
    Run Server With Log Level
    Run Server With Wrong Option
    [Teardown]    Process.Terminate All Processes

*** Keywords ***
Run Server
    ${handle}    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/c3qo
    Process.Process Should Be Running    ${handle}
    Builtin.Sleep    1
    Process.Terminate Process    ${handle}
    Process.Process Should Be Stopped    ${handle}

Run Server With Help
    # Should return immediately
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -h
    Builtin.Should Be True    ${result.rc} == ${0}

Run Server With Log Level
    #
    # With normal log level
    #
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -l    2
    Builtin.Sleep    1
    Process.Terminate Process
    #
    # Corrupted log level (strtol failure)
    #
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -l    300000000000000000000000000000000000
    Builtin.Sleep    1
    Process.Terminate Process

Run Server With Wrong Option
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -z    toto
    Builtin.Should Be True    ${result.rc} != ${0}
