![alt text](https://github.com/secureailabs/Development/blob/main/Logo.jpg?raw=true)

# Development
This is where the source code for all internal and external facing artifacts get maintained and built.

## Building for source

Open a terminal and clone the git repository:
```sh
$ git clone https://github.com/secureailabs/Development.git
```

Generate executables in the following directories using make:

| Directory | Executable |
| ------ | ---------- |
| Development/Milestone2/InternalTools/DatabaseTools/ | DatabaseTools |
| Development/Milestone2/WebService/DatabaseGateway/ | DatabaseGateway |
| Development/Milestone2/WebService/RestApiPortal/ | RestApiPortal |

# Installing and Configuring MongoDB

Run install.sh to install on configure MongoDB Community Edition, MongoDB C Driver, and MongoDB C++ Driver on Ubuntu 20.04.

## Running the Script

Open a terminal and add install.sh as an executable:
```sh
sudo chmod +x install.sh
```

Run the script:
```sh
./install.sh
```

# Testing Login Transaction

Change working directory to Binary:
```sh
$ cd Development/Milestone2/Binary
```

Run the database too (DatabaseTools) and add user(s) to the database:
```sh
$ ./DatabaseTools
```

Start the RestApiPortal server:
```sh
$ ./RestApiPortal
```

Start the DatabaseGateway server:
```sh
$ ./DatabaseGateway
```

Use postman to make an Http request:
* Verb: POST
* Resource: https://localhost:6200/SAIL/AuthenticationManager/User/Login
* Params: Add entries for Email and Password

If the login is successful, the RestApiPortal will return an Eosb otherwise it will return error code.

Once done, stop the servers.
