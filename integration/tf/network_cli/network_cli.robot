*** Settings ***
Library    Process

*** Variables ***
${c3qo_identity}    toto

*** Test Cases ***
Network CLI Errors
    [Documentation]    Check failure cases
    #
    # Wrong main option (anyway, asking for help should be forbidden)
    #
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/ncli    -h
    Builtin.Should Be True    ${result.rc} != ${0}
    #
    # Wrong protobuf option
    #
    Send Protobuf Command    dummy -w    ${1}

Remote Block Management
    [Documentation]    Remotely control a block life cycle
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
    #
    # Terminate
    #
    Builtin.Sleep    1s
    [Teardown]    Teardown Server

*** Keywords ***
Setup Server
    [Documentation]    Start c3qo server
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/proxy    -i    ${c3qo_identity}    alias=proxy
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -i    ${c3qo_identity}    alias=c3qo

Teardown Server
    [Documentation]    Stop c3qo server
    Process.Terminate Process    c3qo
    Process.Terminate Process    proxy    kill=${True}

Send Protobuf Command
    [Arguments]    ${command}    ${expected_rc}=${0}
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/ncli    -i    ${c3qo_identity}    -A    ${command}
    Builtin.Should Be True    ${result.rc} == ${expected_rc}
