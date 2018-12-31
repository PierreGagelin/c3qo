

*** Settings ***
Library    Process

*** Variables ***
${c3qo_identity}    toto

*** Test Cases ***
Network CLI Errors
    [Documentation]    Startup failure cases
    # Asking for help should be forbidden
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/ncli    -h
    Builtin.Should Be True    ${result.rc} != ${0}

    # Wrong command type
    Send Protobuf Command    wrong_type    dummy    ${1}

    # Wrong protobuf option
    Send Protobuf Command    add         dummy -w    ${1}
    Send Protobuf Command    start       dummy -w    ${1}
    Send Protobuf Command    stop        dummy -w    ${1}
    Send Protobuf Command    del         dummy -w    ${1}
    Send Protobuf Command    bind        dummy -w    ${1}
    Send Protobuf Command    hook_zmq    dummy -w    ${1}

    # Configure unknown block
    Start Proxy
    Start C3qo
    Send Protobuf Command    hook_zmq    dummy -i 42 -c -t 0 -n dummy -a dummy    ${1}
    [Teardown]    Process.Terminate All Processes

C3qo Errors
    [Documentation]    Startup failure cases
    # Asking for help should be forbidden
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -h
    Builtin.Should Be True    ${result.rc} != ${0}

    # Unknown option
    ${result}    Process.Run Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -z    toto
    Builtin.Should Be True    ${result.rc} != ${0}

Remote Application Management
    [Documentation]    Remotely manage the c3qo application
    # Start the proxy and a c3qo instance
    Start Proxy
    Start C3qo

    # Stop c3qo via network CLI
    Send Protobuf Command    term    dummy

    # c3qo should be terminated gracefully
    ${result}    Process.Wait For Process    handle=c3qo    timeout=1 s
    BuiltIn.Should Be True    ${result.rc} == ${0}
    [Teardown]    Process.Terminate All Processes

Remote Block Management
    [Documentation]    Remotely manage blocks
    # Start the proxy and a c3qo instance
    Start Proxy
    Start C3qo

    # Life cycle of a block
    Send Protobuf Command    add      dummy -i 0 -t hello
    Send Protobuf Command    start    dummy -i 0
    Send Protobuf Command    stop     dummy -i 0
    Send Protobuf Command    del      dummy -i 0

    # Bind two blocks
    Send Protobuf Command    add     dummy -i 1 -t hello
    Send Protobuf Command    add     dummy -i 2 -t hello
    Send Protobuf Command    bind    dummy -i 1 -p 0 -d 2

    # Configure a ZeroMQ hook
    Send Protobuf Command    add         dummy -i 3 -t hook_zmq
    Send Protobuf Command    hook_zmq    dummy -i 3 -c -t 0 -n name -a tcp://192.168.0.1:7777

    [Teardown]    Process.Terminate All Processes

*** Keywords ***
Start Proxy
    [Documentation]    Start the ZeroMQ proxy to connect network CLI to every c3qo instances
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/proxy    -i    ${c3qo_identity}    alias=proxy

Stop Proxy
    [Documentation]    Stop the ZeroMQ proxy, it freezes if a send is in progress
    [Timeout]    1 s
    Process.Terminate Process    handle=proxy

Start C3qo
    [Documentation]    Start the c3qo instance
    Process.Start Process    /tmp/c3qo-0.0.7-local/bin/c3qo    -i    ${c3qo_identity}    alias=c3qo

Stop C3qo
    [Documentation]    Stop the c3qo instance
    Process.Terminate Process    handle=c3qo

Send Protobuf Command
    [Arguments]  ${type}  ${opt}  ${expected_rc}=${0}
    ${result}  Process.Run Process  /tmp/c3qo-0.0.7-local/bin/ncli  -i  ${c3qo_identity}  -t  ${type}  -o  ${opt}
    Builtin.Should Be True  ${result.rc} == ${expected_rc}
