*** Settings ***
Library    ../../lib/c3qo.py
Library    ../../lib/ncli.py

*** Test Cases ***
Check Command Raw
    Server Run
    ncli.send    raw    dummy -p "1 10 trans_pb"
    ncli.send    raw    dummy -p "5 10"
    ncli.send    raw    dummy -p "1 11 project_euler"
    ncli.send    raw    dummy -p "5 11"
    Builtin.Sleep    1s
    Server Stop

Check Command Protobuf
    Server Run
    ncli.send    proto    dummy -i 20 -t add -a hello
    ncli.send    proto    dummy -i 20 -t conf -a ZIGOUILLATOR3000
    ncli.send    proto    dummy -i 20 -t bind -a 2:10
    ncli.send    proto    dummy -i 20 -t start
    ncli.send    proto    dummy -i 20 -t stop
    ncli.send    proto    dummy -i 20 -t del
    Builtin.Sleep    1s
    Server Stop

*** Keywords ***
Server Is Absent
    ${present}    c3qo.Check Present
    Builtin.Run Keyword If    ${present} is ${True}    Builtin.FAIL

Server Is Present
    ${present}    c3qo.Check Present
    Builtin.Run Keyword If    ${present} is ${False}    Builtin.FAIL

Server Run
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Absent
    c3qo.Add    key=server_007
    c3qo.Run    key=server_007
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Present

Server Stop
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Present
    c3qo.Stop
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Absent
