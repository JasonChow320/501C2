### To run

run the command line "python client.py"

### to use

When connected to the c2, note you have to change the ip config of the file (ip for the c2) to have the client working. Once you are connected to the c2, you can run any commands available

### commands

commands include: 
1) sql -> sees all available agents
2) remote *ip* *commands* -> queue a command to an agent/implant at *ip* address, the commands include steal (chromium stealer), kill (to kill the agent), and any powershell/command prompt shells will work
3) getData *ip* -> get the outputs of the commands executed by the implant
