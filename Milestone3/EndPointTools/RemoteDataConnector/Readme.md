# RemoteDataConnector
The tool provides an interface for the data onwer to upload the dataset to the freshly initialized computational nodes waiting for datasets.

# Usage
1. Create a folder (lets say 'Datasets') which only contains the dataset files(.csvp).
2. Either run the tool in interactive mode where the tool will promt for each parameter or run it in by providing commandLine paramters
    ```
        ./RemoteDataConnector --PortalIp=<RestPortalIp> --Port=<RestPortalPort> --User=<DataOwnerEmail> --Password=<Password> --Folder=<path to dataset directory>
    ```
    Example:
    ```
        ./RemoteDataConnector --PortalIp=40.76.22.246 --Port=6200 --User=data@dataowner.com --Password=SailPassword@123 --Folder=./Datasets
    ```

