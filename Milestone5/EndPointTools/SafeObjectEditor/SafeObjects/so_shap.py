import shap
import matplotlib.pyplot as pl

#input1 __model
#input1 __df

#output __pic


print("shap data")

shap.initjs()
explainer = shap.TreeExplainer(__model)
shap_values = explainer.shap_values(__df)
shap.summary_plot(shap_values, __df, show=False)

__pic = pl.gcf()
