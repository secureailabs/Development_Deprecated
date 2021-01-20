# CLI FRONTEND FOR REMOTE RESEARCHERS

The CLI interface supported commands are as follows:

1. help  
usage: `>>> help`  
output: help messages  

2. quit  
usage: `>>> quit`  
output: quit the CLI  

3. run
usage: `>>> run -t <script type> -m <module name> -f <function signature> -n <job name>`  
example: `>>> run -t python -m sail_logistic_regression -f logistic_regression(0, [0,1,2,3], 4) -n test`  
This command will call logistic_regression function in sail_logistic_regression module.  
The parameters of logstic_regression function are tableID, feature column sets and target column.  
output: The output will give the job ID, which can be used to query the result of the job.  

4. check
usage: `>>> check -n <job ID>`  
output: the result of the job identified by job ID.  

5. inspect
usage: `>>> inspect -t <inspect target type>`  
The target type can be either `job` or `dataset`.  
output: The jobs running or datasets available.  
