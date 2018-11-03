*** Settings ***
Library    ../../lib/c3qo.py

*** Variables ***
${VERSION}    0.0.7    # Just here to keep "Variables"

*** Test Cases ***
Check Server Startup
    Run Server
    Run Server With Help
    Run Server With Log Level
    Run Server With Wrong Option

*** Keywords ***
Server Is Absent
    ${present}    c3qo.Check Present
    Builtin.Run Keyword If    ${present} is ${True}    Builtin.FAIL

Server Is Present
    ${present}    c3qo.Check Present
    Builtin.Run Keyword If    ${present} is ${False}    Builtin.FAIL

Run Server
    c3qo.Add    key=server_007
    c3qo.Run    key=server_007
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Present
    c3qo.Stop

Run Server With Help
    c3qo.Add    key=server_007    helper=${True}
    c3qo.Run    key=server_007
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Absent

Run Server With Log Level
    #
    # With normal log level
    c3qo.Add    key=server_007    log=LOG_NONE
    c3qo.Run    key=server_007
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Present
    c3qo.Stop
    #
    # Corrupted log level (strtol failure)
    c3qo.Add    key=server_007    arg=-l 300000000000000000000000000000000000
    c3qo.Run    key=server_007
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Present
    c3qo.Stop

Run Server With Wrong Option
    c3qo.Add    key=server_007    arg=-z toto
    c3qo.Run    key=server_007
    Builtin.Wait Until Keyword Succeeds    3s    1s    Server Is Absent
