[![.github/workflows/build.yml](https://github.com/secureailabs/Development/actions/workflows/build.yml/badge.svg)](https://github.com/secureailabs/Development/actions/workflows/build.yml)
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
| Development/Milestone2/EndPointTools/RestPortalTester/ | RestPortalTester |

# Installing and Configuring MongoDB

Run install.sh to install and configure MongoDB Community Edition, MongoDB C Driver, and MongoDB C++ Driver on Ubuntu 20.04.

## Running the Script

Open a terminal and add install.sh as an executable:
```sh
sudo chmod +x install.sh
```

Run the script:
```sh
./install.sh
```

# Testing

Change working directory to Binary:
```sh
$ cd Development/Milestone2/Binary
```

Run the database tool (DatabaseTools) and add user(s) to the database:
```sh
$ ./DatabaseTools
```

Start the RestApiPortal server (in Milestone5 the options --DatabaseServerIp and --DatabaseServerPort can be provided to specify a remote database gateway):
```sh
$ ./RestApiPortal
```

Start the DatabaseGateway server:
```sh
$ ./DatabaseGateway
```

Run the interactive client (RestPortalTester) to test login, event registration, and event enumeration:
```sh
$ ./RestPortalTester
```

You can also use Postman to make Http requests to the RestApiPortal. An example request is as following:
* Verb: POST
* Resource: https://localhost:6200/SAIL/AuthenticationManager/User/Login
* Params: Add entries for Email and Password

If the login is successful, the RestApiPortal will return an Eosb otherwise it will return an error code.

Once done, stop the servers.
