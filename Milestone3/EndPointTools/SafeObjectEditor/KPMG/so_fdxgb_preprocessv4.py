import datetime as dt
import pandas as pd
import numpy as np
import scipy

agreement_df = __data0
contact_df = __data1
payment_df = __data2

contact_df = contact_df.drop(["ContactID", "CreateReasonGUID", "CancelReasonGUID", "Age"], axis=1)
agreement_df = agreement_df.drop(["ContactGUID", "AgreementID", "ChannelName", "FutureCancelDate",
                                  "ContactID"], axis=1)
if "Membership" in payment_df.columns:
    print("Renaming \"Membership\"")
    payment_df.rename(columns={'Membership': 'Medlemskab'}, inplace=True)

payment_df = payment_df[["PaymentGUID", "PaymentID", "ContactGUID", "AgreementGUID", "AgreementtypeGUID", "PaymentMethod",
                        "PaymentMethodName", "PaymentDate", "PaidAmount", "ChargedDate", "ChargedAmount", "PaymentStatus",
                        "PaymentStatusName", "DirectMail", "Donation", "Medlemskab", "Product", "Lottery"]]

payment_df = payment_df.drop(["PaymentID", "AgreementtypeGUID", "PaymentMethodName", "PaymentStatusName"], axis=1)

if "DontKnow" in payment_df.columns:
    payment_df = payment_df.drop(["DontKnow"], axis = 1)

if payment_df["Donation"].dtype!=bool:
    payment_df["Donation"] = payment_df["Donation"].astype(bool)

if payment_df["Product"].dtype!=bool:
    payment_df.loc[payment_df["Product"]!='True', "Product"]=False
    payment_df["Product"]=payment_df["Product"].astype(bool)

if payment_df["Medlemskab"].dtype!=bool:
    payment_df["Medlemskab"]=payment_df["Medlemskab"].astype(bool)

merged_df = payment_df.merge(agreement_df, on="AgreementGUID", how="left").merge(contact_df,
                                                                                 on="ContactGUID", how = "left")
print("Size after original merge:", merged_df.shape)

# Cancel reasons to remove
nchurn_reasons_SCL = ["690a05e3-607e-ea11-a811-000d3aba7e84", "701b19de-607e-ea11-a811-000d3ada453a", "7a1b19de-607e-ea11-a811-000d3ada453a", 
                     "5da5d93a-eebc-ea11-a812-000d3ada453a", "c18c0ec6-d0af-ea11-a812-000d3ada453a", "b0ece5ad-d0af-ea11-a812-000d3ada453a", 
                      "6f0a05e3-607e-ea11-a811-000d3aba7e84", "6f1b19de-607e-ea11-a811-000d3ada453a", "6e0a05e3-607e-ea11-a811-000d3aba7e84",
                      "497411e4-607e-ea11-a811-000d3ada453a", "721b19de-607e-ea11-a811-000d3ada453a", "4d7411e4-607e-ea11-a811-000d3ada453a",
                      "7d1b19de-607e-ea11-a811-000d3ada453a", "7c1b19de-607e-ea11-a811-000d3ada453a", "711b19de-607e-ea11-a811-000d3ada453a", 
                      "e067123c-7c2a-eb11-a813-000d3ada453a", "6b0a05e3-607e-ea11-a811-000d3aba7e84", "a88506c7-8af6-ea11-a815-000d3ada453a",
                      "4c7411e4-607e-ea11-a811-000d3ada453a", "791b19de-607e-ea11-a811-000d3ada453a", "751b19de-607e-ea11-a811-000d3ada453a",
                      "731b19de-607e-ea11-a811-000d3ada453a", "771b19de-607e-ea11-a811-000d3ada453a", "6d0a05e3-607e-ea11-a811-000d3aba7e84",
                     "4a7411e4-607e-ea11-a811-000d3ada453a"]

nchurn_reasons_GFNG = ["395742a5-dc04-e811-8116-5065f38aea11", "992dee8c-8906-e811-8111-5065f38b4641", "59ae8760-1155-e911-a95b-000d3ab6413d",
                       "715742a5-dc04-e811-8116-5065f38aea11", "705f85e0-7549-e911-a95a-000d3ab6488a", "5b48a520-4ea1-e811-8139-5065f38b4641",
                       "4b5742a5-dc04-e811-8116-5065f38aea11", "64c7e893-aa23-e911-a955-000d3ab6488a", "8db566eb-a24d-ea11-a812-000d3ab11e51",
                       "f0b7e130-5677-eb11-a812-000d3ab25083", "f47f6349-d1cb-e811-8141-5065f38b5621", "7d2626f9-d542-e811-8119-5065f38b5621",
                       "9bd72131-89b7-ea11-a812-000d3aba8599", "27950b8a-4079-e811-8129-5065f38b5621", "6ed69b43-d642-e811-8119-5065f38b5621",
                       "2565143c-d91d-eb11-a813-000d3aba8599"]

nchurn_reasons = nchurn_reasons_SCL + nchurn_reasons_GFNG

# Remove data with unrelevant cancel reasons
nchurn_contacts = merged_df[merged_df["Cancelreason"].isin(nchurn_reasons)]["ContactGUID"].unique()
merged_df = merged_df[~merged_df["ContactGUID"].isin(nchurn_contacts)]
print("Size after removing unvalid cancel reasons:", merged_df.shape)

# Remove payments without payment date
merged_df = merged_df[~merged_df["PaymentDate"].isna()]
print("Size after removing payments without date:", merged_df.shape)

print("Unique person IDs:", merged_df.ContactGUID.nunique())

# Convert PaymentDate to date type
merged_df["PaymentDate"] = pd.to_datetime(merged_df["PaymentDate"])
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])
merged_df["CancelDate"] = pd.to_datetime(merged_df["CancelDate"])

# Change all payment days to the first day of the given month
merged_df["PaymentDate"] = merged_df.PaymentDate.apply(lambda dt: dt.replace(day=1))
merged_df["Startdate"] = merged_df.Startdate.apply(lambda dt: dt.replace(day=1))
merged_df["CancelDate"] = merged_df.CancelDate.apply(lambda dt: dt.replace(day=1))

print("Number of unique persons who ever cancelled agreement:")
merged_df[merged_df.CancelDate.notnull()].ContactGUID.nunique()

extended_df = merged_df.copy()

# Save minimum and maximum values of the PaymentDates
date_df = extended_df.groupby("ContactGUID").agg(
    PaymentDate_min=('PaymentDate', np.min), PaymentDate_max=('PaymentDate', np.max))

ids = extended_df.groupby('ContactGUID', group_keys=False).first().index.tolist()
date_range = pd.date_range(
    extended_df["PaymentDate"].min().date(), extended_df["PaymentDate"].max().date(), freq=pd.offsets.MonthBegin(1))

df = pd.DataFrame(index=ids, columns=date_range)
df['temp']=(' '.join(df.columns.astype(str)))
df = df['temp'].str.split(expand=True)
df["Dates"] = df[df.columns].values.tolist()
df = df[["Dates"]]
df = df.explode("Dates").reset_index().rename(columns = {'index':'ContactGUID', 'Dates':'PaymentDate'})
df["PaymentDate"] = pd.to_datetime(df["PaymentDate"])

extended_df = extended_df.merge(df, on=["PaymentDate", "ContactGUID"],
                          how='outer', indicator=True).reset_index().sort_values(by=["ContactGUID", "PaymentDate"])

extended_df["Cancelreason"] = extended_df["Cancelreason"].fillna("00000000-0000-0000-0000-000000000000")

# Add payment minimum dates to know the range for a given person
extended_df["PaymentDate_min"] = extended_df.ContactGUID.apply(lambda x: date_df.loc[x]["PaymentDate_min"])

# Remove erlier rows than the first payment of the person
payment_in_range = np.where(extended_df['PaymentDate'] >= extended_df["PaymentDate_min"], True, False)
extended_df = extended_df[payment_in_range]

print("Size with dummy rows only inside the range:", extended_df.shape)

# Get the date of the first churned agreement per person
churned_df = extended_df[extended_df["CancelDate"].isna() == False].groupby(
    "ContactGUID", as_index = False)["CancelDate"].min()

extended_df = extended_df.merge(churned_df, on="ContactGUID", how="left")

feature_df = extended_df.copy()

# Is the payment associated with an agreement
feature_df["payment_on_agreement"] = np.where(feature_df["AgreementGUID"].isna(), False, True)

# Fix dot instead of comma if PaidAmount includes any
try:
    feature_df["PaidAmount"] = feature_df["PaidAmount"].str.replace(',','.')
except:
    print("Objects are not strings, cannot replace comma with dot.")

# Convert datatypes
feature_df["Startdate"] = pd.to_datetime(feature_df["Startdate"])
feature_df["CancelDate_y"] = pd.to_datetime(feature_df["CancelDate_y"])
feature_df["ChargedDate"] = pd.to_datetime(feature_df["ChargedDate"])

feature_df["PaidAmount"] = pd.to_numeric(feature_df["PaidAmount"])

# Churn flag
feature_df['is_churned'] = False
feature_df['is_churned'] = np.where((feature_df['CancelDate_y'].dt.month <= feature_df["PaymentDate"].dt.month) &
                                   (feature_df['CancelDate_y'].dt.year <= feature_df["PaymentDate"].dt.year), True, False)

# Remove payments after churn date
feature_df = feature_df[~feature_df["index"].isin(
    feature_df[feature_df["PaymentDate"] > feature_df["CancelDate_y"]]["index"])]

print("Size after removing payments after a churn has occured:", feature_df.shape)

# Months since start
feature_df["months_since_start"] = (feature_df["PaymentDate"]- feature_df["Startdate"])/np.timedelta64(1, 'M')

# Months since last payment
feature_df = feature_df.sort_values(by=[ "ContactGUID", "PaymentDate"])
feature_df["months_since_last_payment"] = feature_df.groupby(
    ["ContactGUID", "AgreementGUID"])["PaymentDate"].diff()/np.timedelta64(1, 'M')

# Missing payment
feature_df["missing_payment"] = np.where(((feature_df["PaymentStatus"] == 110690003) & (feature_df["PaidAmount"] == 0)) |
                                       ((feature_df["PaymentStatus"] == 110690004) & (feature_df["PaidAmount"] == 0)), True,
                                        False)

# Add month as feature
feature_df["PaymentMonth"] = feature_df["PaymentDate"].dt.month

# Months since missing payment feature
grouped = feature_df.groupby("ContactGUID")
result = []
holder = ''

for group_name, group in grouped:
    holding = False
    for row_index, row in group.iterrows():
        if row['missing_payment'] == True:
            holding = True
            holder = row["PaymentDate"]
            result.append(0) 
        elif row["missing_payment"] == False:
            if holding == False:
                result.append(0)
            if holding == True:
                result.append((row['PaymentDate'] - holder)/np.timedelta64(1, 'M'))
                
feature_df["months_since_missing_payment"] = result

feature_df["is_churned"].value_counts()

one_hot_list = ["PaymentMethod", "PaymentStatus", "ChannelD", "AgreementType", "CreateReason", "PaymentMonth"]
onehot_df = pd.get_dummies(feature_df, columns=one_hot_list)

# Removing the .0 from the "ChannelID..." column names because it raises error later with the explainerdashboard
for n in onehot_df.columns:
    if ".0" in n:
        onehot_df = onehot_df.rename(columns={n:n.replace('.0', '')})

agg_dict = {
    'PaidAmount': 'sum',
    'AgreementGUID' : 'count',
    'PaymentGUID' : 'count',
    'CancelDate_y': 'min',
    'Donation': 'sum',
    'Medlemskab': 'sum',
    'Product': 'sum',
    'Lottery': 'sum',
    'payment_on_agreement': 'sum',
    'months_since_start': ['min', 'max'],
    'months_since_last_payment' : ['min', 'max'],
    'months_since_missing_payment' : ['min', 'max'],
    'missing_payment' : 'sum',
    'is_churned': 'max'
}

for one_hot_col in one_hot_list:
    tmp_dict = {x: 'sum' for x in onehot_df.columns if one_hot_col in x}
    agg_dict.update(tmp_dict)

# Group and aggregate
agg_df = onehot_df.set_index("PaymentDate").groupby(["ContactGUID", pd.Grouper(freq=pd.offsets.MonthBegin(1))]).agg(agg_dict)

agg_df["is_churned"].value_counts()

agg_df = agg_df.reset_index()
agg_df.columns = ["_".join(x) for x in agg_df.columns.ravel()]

print("Size before aggregation:", onehot_df.shape)
print("Size after aggregation:", agg_df.shape)

# Churned within three months
agg_df["is_churned_within3m"] = np.where((agg_df['CancelDate_y_min'].dt.year <= agg_df["PaymentDate_"].dt.year) &
                                   (agg_df['CancelDate_y_min'].dt.month <= agg_df["PaymentDate_"].dt.month+3), True, False)

# Cummulated total paid amount
agg_df["cumulated_paid_amount"] = agg_df.groupby(["ContactGUID_"])["PaidAmount_sum"].cumsum()

# Convert sum values to numeric (there are some boolean for some reason)
agg_df["Donation_sum"] = pd.to_numeric(agg_df["Donation_sum"])
agg_df["Medlemskab_sum"] = pd.to_numeric(agg_df["Medlemskab_sum"])
agg_df["Product_sum"] = pd.to_numeric(agg_df["Product_sum"])

agg_df["is_churned_within3m"].value_counts()

# Filling up NaN-s with 0:
agg_df["PaidAmount_sum"] = agg_df["PaidAmount_sum"].fillna(0) 
agg_df["PaymentGUID_count"] = agg_df["PaymentGUID_count"].fillna(0)
agg_df["cumulated_paid_amount"] = agg_df["cumulated_paid_amount"].fillna(0)

# Cumulated paid amount in the last three months
roll_df = agg_df.copy()

roll_df["cumulated_amount_3months"] = roll_df[["PaidAmount_sum"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
roll_df["cumulated_amount_6months"] = roll_df[["PaidAmount_sum"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())

# Number of payments made for the last 3m, last 6m and total
roll_df["payment_count_last3m"] = roll_df[["PaymentGUID_count"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
roll_df["payment_count_last6m"] = roll_df[["PaymentGUID_count"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())

roll_df["payment_count_total"] = roll_df["PaymentGUID_count"].groupby(
    roll_df["ContactGUID_"]).cumsum()

roll_df["is_churned_within3m"] = roll_df["is_churned_within3m"].astype(int)

# Filling up NaN-s with 0:
roll_df["cumulated_amount_3months"] = roll_df["cumulated_amount_3months"].fillna(0)
roll_df["cumulated_amount_6months"] = roll_df["cumulated_amount_6months"].fillna(0)
roll_df["payment_count_last3m"] = roll_df["payment_count_last3m"].fillna(0)
roll_df["payment_count_last6m"] = roll_df["payment_count_last6m"].fillna(0)

# Drop features that are not needed/not explore yet
final_df = roll_df.drop(["CancelDate_y_min", "is_churned_max", "Lottery_sum"], axis=1)

#import math

class_count = final_df["is_churned_within3m"].value_counts().to_list()
print(class_count)
#print("Class imbalance:\n\t Not churned in 3 months:", class_count[0]/sum(class_count)*100)
#print("\t Churned in 3 months:", class_count[1]/sum(class_count)*100)
#print("Ratio:", class_count[0]/class_count[1])
#print("Ratio squared:", math.sqrt(class_count[0]/class_count[1]))

monthly_churn_df = final_df[["is_churned_within3m"]].groupby(final_df["PaymentDate_"]).sum()
monthly_churn_df["total_in_month"] = final_df[["is_churned_within3m"]].groupby(final_df["PaymentDate_"]).size()
monthly_churn_df["not_churned"] = monthly_churn_df["total_in_month"] - monthly_churn_df["is_churned_within3m"]
monthly_churn_df = monthly_churn_df.drop(columns="total_in_month")

final_df = final_df.set_index(["ContactGUID_", "PaymentDate_"])

__output = final_df

print(final_df.shape)
print(final_df.columns)





