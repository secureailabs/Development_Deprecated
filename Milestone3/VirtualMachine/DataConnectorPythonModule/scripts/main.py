from scripts import sail_xgboost
from scripts import sail_logistic_regression
import sys

eval(str(sys.argv[1]))

sys.stdout.flush()
sys.stderr.flush()
