*** Settings ***
Library    Process

*** Variables ***
${c3qo_identity}    toto

*** Test Cases ***
Network CLI Errors
    [Documentation]    Check startup failure cases
    #
    # Asking for help should be forbidden
    #
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/ncli    -h
    Builtin.Should Be True    ${result.rc} != ${0}
    #
    # Wrong protobuf option
    #
    Send Protobuf Command    dummy -w    ${1}

C3qo Errors
    [Documentation]    Check startup failure cases
    #
    # Asking for help should be forbidden
    #
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -h
    Builtin.Should Be True    ${result.rc} != ${0}
    #
    # Unknown option
    #
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -z    toto
    Builtin.Should Be True    ${result.rc} != ${0}

Remote Application Management
    [Documentation]    Remotely manage the c3qo application
    [Setup]    Setup Server
    Stop Server
    [Teardown]    Teardown Server

Check Server Startup
    [Documentation]    Various ways to start c3qo server
    [Setup]    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/proxy
    #
    # Minimal start
    #
    ${args}    BuiltIn.Create List
    Start Server    ${args}
    Stop Server
    [Teardown]    Teardown Server

Remote Block Management
    [Documentation]    Remotely manage a block life cycle
    [Setup]    Setup Server
    #
    # Send the commands to do a life cycle
    #
    Send Protobuf Command    dummy -i 10 -t add -a hello
    Send Protobuf Command    dummy -i 20 -t add -a hello
    Send Protobuf Command    dummy -i 20 -t bind -p 2 -d 10
    Send Protobuf Command    dummy -i 20 -t start
    Send Protobuf Command    dummy -i 20 -t stop
    Send Protobuf Command    dummy -i 20 -t del
    #
    # Send an unknown command
    #
    Send Protobuf Command    dummy -i 20 -t wrong_type    ${1}
    [Teardown]    Teardown Server

*** Keywords ***
Setup Server
    [Documentation]    Start c3qo server
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/proxy    -i    ${c3qo_identity}    alias=proxy
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -i    ${c3qo_identity}    alias=c3qo

Teardown Server
    [Documentation]    Stop c3qo server
    Process.Terminate All Processes

Start Server
    [Arguments]    ${args}
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -i    ${c3qo_identity}    @{args}    alias=c3qo

Stop Server
    Send Protobuf Command    dummy -t term
    #
    # Process should be terminated gracefully
    #
    ${result}    Process.Wait For Process    handle=c3qo    timeout=1 s
    BuiltIn.Should Be True    ${result.rc} == ${0}

Send Protobuf Command
    [Arguments]    ${command}    ${expected_rc}=${0}
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/ncli    -i    ${c3qo_identity}    -A    ${command}
    Builtin.Should Be True    ${result.rc} == ${expected_rc}
