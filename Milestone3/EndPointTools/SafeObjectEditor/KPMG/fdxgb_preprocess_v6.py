print("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", flush=True)

import datetime as dt
import pandas as pd
import numpy as np
import math

agreement_df = __data0
contact_df = __data1
payment_df = __data2
agreement_map_df = __data3
create_map_df = __data4

# ### Drop columns

# In[8]:
print("1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", flush=True)


contact_df = contact_df.drop(["ContactID", "CreateReasonGUID", "CancelReasonGUID"], axis=1)

print("2222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222", flush=True)

# In[9]:


# Make 100+ and negative ages to NaNs
contact_df.loc[(contact_df['Age'] > 100) | (contact_df['Age'] < 0)] = np.nan

print("3333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333", flush=True)

# In[11]:


# Rename Membership of it can be found among the column names
if "Membership" in payment_df.columns:
    print("Renaming \"Membership\"")
    payment_df.rename(columns={'Membership': 'Medlemskab'}, inplace=True)
print("4444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444", flush=True)

#if "DontKnow" in payment_df.columns:
#    payment_df = payment_df.drop(["Dontknow"], axis = 1)
#print("5555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555", flush=True)

#if payment_df["Donation"].dtype!=bool:
#    payment_df.loc[payment_df["Product"]!='True', "Product"]=False
#    payment_df["Donation"] = payment_df["Donation"].astype(bool)

if payment_df["Product"].dtype!=bool:
    payment_df.loc[payment_df["Product"]!='True', "Product"]=False
    payment_df["Product"]=payment_df["Product"].astype(bool)

#if payment_df["Medlemskab"].dtype!=bool:
#    payment_df.loc[payment_df["Product"]!='True', "Product"]=False
#    payment_df["Medlemskab"]=payment_df["Medlemskab"].astype(bool)

# In[10]:
payment_cols = ["PaymentGUID", "PaymentID", "ContactGUID", "AgreementGUID", "AgreementtypeGUID", "PaymentMethod",
                "PaymentMethodName", "PaymentDate", "PaidAmount", "ChargedDate", "ChargedAmount", "PaymentStatus",
                "PaymentStatusName", "DirectMail", "Donation", "Medlemskab", "Product", "Lottery"]

if payment_df.columns[0] not in payment_cols:
    payment_df = payment_df.set_axis(payment_cols, axis=1, inplace=False)

agreement_df = agreement_df.drop(["ContactGUID", "AgreementID", "ChannelName", "FutureCancelDate",
                                  "ContactID"], axis=1)
print("66666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666", flush=True)


# In[12]:


payment_df = payment_df.drop(["PaymentID", "AgreementtypeGUID", "PaymentMethodName", "PaymentStatusName"], axis=1)


# ### Data Merging

# In[13]:
#contact_df = contact_df.drop_duplicates()

merged_df = payment_df.merge(agreement_df, on="AgreementGUID", how="left")
merged_df = merged_df.merge(contact_df, on="ContactGUID", how = "left")
print("Size after original merge:", merged_df.shape)

from sys import getsizeof

print(getsizeof(merged_df), flush=True)

print("777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777", flush=True)

# In[14]:


# Cancel reasons to remove
churn_reasons_SCL = [
    "690a05e3-607e-ea11-a811-000d3aba7e84", "701b19de-607e-ea11-a811-000d3ada453a", "7a1b19de-607e-ea11-a811-000d3ada453a", 
    "5da5d93a-eebc-ea11-a812-000d3ada453a", "c18c0ec6-d0af-ea11-a812-000d3ada453a", "b0ece5ad-d0af-ea11-a812-000d3ada453a", 
    "6f0a05e3-607e-ea11-a811-000d3aba7e84", "6f1b19de-607e-ea11-a811-000d3ada453a", "6e0a05e3-607e-ea11-a811-000d3aba7e84",
    "497411e4-607e-ea11-a811-000d3ada453a", "721b19de-607e-ea11-a811-000d3ada453a", "4d7411e4-607e-ea11-a811-000d3ada453a",
    "7d1b19de-607e-ea11-a811-000d3ada453a", "7c1b19de-607e-ea11-a811-000d3ada453a", "711b19de-607e-ea11-a811-000d3ada453a", 
    "e067123c-7c2a-eb11-a813-000d3ada453a", "6b0a05e3-607e-ea11-a811-000d3aba7e84", "a88506c7-8af6-ea11-a815-000d3ada453a",
    "4c7411e4-607e-ea11-a811-000d3ada453a", "791b19de-607e-ea11-a811-000d3ada453a", "751b19de-607e-ea11-a811-000d3ada453a",
    "731b19de-607e-ea11-a811-000d3ada453a", "771b19de-607e-ea11-a811-000d3ada453a", "6d0a05e3-607e-ea11-a811-000d3aba7e84",
    "4a7411e4-607e-ea11-a811-000d3ada453a"]

nchurn_reasons_GFNG = [
    "395742a5-dc04-e811-8116-5065f38aea11", "992dee8c-8906-e811-8111-5065f38b4641", "59ae8760-1155-e911-a95b-000d3ab6413d",
    "715742a5-dc04-e811-8116-5065f38aea11", "705f85e0-7549-e911-a95a-000d3ab6488a", "5b48a520-4ea1-e811-8139-5065f38b4641",
    "4b5742a5-dc04-e811-8116-5065f38aea11", "64c7e893-aa23-e911-a955-000d3ab6488a", "8db566eb-a24d-ea11-a812-000d3ab11e51",
    "f0b7e130-5677-eb11-a812-000d3ab25083", "f47f6349-d1cb-e811-8141-5065f38b5621", "7d2626f9-d542-e811-8119-5065f38b5621",
    "9bd72131-89b7-ea11-a812-000d3aba8599", "27950b8a-4079-e811-8129-5065f38b5621", "6ed69b43-d642-e811-8119-5065f38b5621",
    "2565143c-d91d-eb11-a813-000d3aba8599"]

nchurn_reasons = churn_reasons_SCL + nchurn_reasons_GFNG


# In[15]:


# Remove data with unrelevant cancel reasons
nchurn_contacts = merged_df[merged_df["Cancelreason"].isin(nchurn_reasons)]["ContactGUID"].unique()
merged_df = merged_df[~merged_df["ContactGUID"].isin(nchurn_contacts)]
print("Size after removing unvalid cancel reasons:", merged_df.shape)

print("77777777777777777777777777777777755555555555555555555555555555555555555555555555555555555555555555555555", flush=True)

# In[16]:


# Remove payments without payment date
merged_df = merged_df[~merged_df["PaymentDate"].isna()]
print("Size after removing payments without date:", merged_df.shape)
print("Unique person IDs:", merged_df.ContactGUID.nunique())

print("77777777777777777777777777777777766666666666666666666666666666666666666666666666666666666666666666666666", flush=True)

# In[17]:


# Convert PaymentDate to date type
merged_df["PaymentDate"] = pd.to_datetime(merged_df["PaymentDate"])
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])
merged_df["CancelDate"] = pd.to_datetime(merged_df["CancelDate"])
merged_df["ChargedDate"] = pd.to_datetime(merged_df["ChargedDate"])
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])

print("777777777777777777777777777777777______________________________________________________________________", flush=True)

# In[18]:


# Change all payment days to the first day of the given month
merged_df["PaymentDate"] = merged_df.PaymentDate.apply(lambda dt: dt.replace(day=1))
merged_df["Startdate"] = merged_df.Startdate.apply(lambda dt: dt.replace(day=1))
merged_df["CancelDate"] = merged_df.CancelDate.apply(lambda dt: dt.replace(day=1))

print("777777777777777777777777777777777888888888888888888888888888888888888888888888888888888888888888888888888", flush=True)

# In[19]:


print("Number of unique persons ever cancelled agreement:", merged_df[merged_df.CancelDate.notnull()].ContactGUID.nunique())


# ### Adding empty rows

# In[20]:


# Add payment minimum and maximum dates to know the range for a given person
merged_df['PaymentDate_min'] = merged_df.groupby('ContactGUID')['PaymentDate'].transform('min')
merged_df['PaymentDate_max'] = merged_df.groupby('ContactGUID')['PaymentDate'].transform('max')

print("77777777777777777777777777777777799999999999999999999999999999999999999999999999999999999999999999999", flush=True)


# In[21]:


# list IDs and create data range that covers everything from the first ever payment until the last
ids = list(merged_df.ContactGUID.unique())
date_range = pd.date_range(
    merged_df["PaymentDate"].min().date(), merged_df["PaymentDate"].max().date(), freq='MS')

print("77777777777777777777777777777777788888888888888888888888888888888885555555555555555555555555555555555", flush=True)


# In[22]:


# Create a dataframe where all unique person is assigned to all the dates from the previous date range
df = pd.DataFrame(index=ids, columns=date_range)
df['temp']=(' '.join(df.columns.astype(str)))
df = df['temp'].str.split(expand=True)
df["Dates"] = df[df.columns].values.tolist()
df = df[["Dates"]]
df = df.explode("Dates").reset_index().rename(columns = {'index':'ContactGUID', 'Dates':'PaymentDate'})
df["PaymentDate"] = pd.to_datetime(df["PaymentDate"])

print("88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888", flush=True)

# In[23]:


# Merge the dummy dates together with the original data to fill the missing months
merged_df = merged_df.merge(df, on=["PaymentDate", "ContactGUID"],
                          how='outer', indicator=True).reset_index().sort_values(by=["ContactGUID", "PaymentDate"])


# In[24]:


# Is the payment associated with an agreement
merged_df["payment_on_agreement"] = np.where(merged_df["AgreementGUID"].isna(), False, True)


# In[25]:


# Change payment_on_agreement flag to True if the person has ever had an agreement (assign it to the dummy rows)
merged_df['payment_on_agreement'] = merged_df.groupby('ContactGUID')['payment_on_agreement'].transform('max')


# In[26]:


# Copy mimimum and maximum payment dates to the new cells
merged_df["PaymentDate_min"] = merged_df.sort_values(
    ['ContactGUID','PaymentDate_min'],ascending=True).groupby('ContactGUID').PaymentDate_min.ffill()
merged_df["PaymentDate_max"] = merged_df.sort_values(
    ['ContactGUID','PaymentDate_max'],ascending=True).groupby('ContactGUID').PaymentDate_max.ffill()


# In[27]:


# Remove erlier rows than the first payment of the person
payments_not_earlier = np.where(merged_df['PaymentDate'] >= merged_df["PaymentDate_min"], True, False)
merged_df = merged_df[payments_not_earlier]


# In[28]:


# Remove later rows than the last payment of the person if there is no agreement
# (we cannot know if the person has already cancelled the agreement or not...)
payment_after_without_agreement = np.where((merged_df['PaymentDate'] > merged_df["PaymentDate_max"])
                                            & ~merged_df["payment_on_agreement"], True, False)
merged_df = merged_df[~payment_after_without_agreement]


# In[29]:


print("Size with dummy rows only inside the range:", merged_df.shape)


# ### Add churn flags

# In[30]:


# Get the date of the first churned agreement per person
churned_df = merged_df[merged_df["CancelDate"].isna() == False].groupby(
    "ContactGUID", as_index = False)["CancelDate"].min()

print("9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999", flush=True)

# In[31]:


merged_df = merged_df.merge(churned_df, on="ContactGUID", how="left")


# In[32]:


# Churn flag for those who have a cancel date with not older date than the payment date OR those who have future cancel registered. 
merged_df['is_churned'] = False
merged_df['is_churned'] = np.where(((merged_df['CancelDate_y'].dt.month <= merged_df["PaymentDate"].dt.month) &
                                   (merged_df['CancelDate_y'].dt.year <= merged_df["PaymentDate"].dt.year)) | 
                                    merged_df.FutureCancel == True, True, False)


# In[33]:


# Remove payments after churn date
merged_df = merged_df[~merged_df["index"].isin(
    merged_df[merged_df["PaymentDate"] > merged_df["CancelDate_y"]]["index"])]


# In[34]:


print("Size after removing payments after a churn has occured:", merged_df.shape)


# In[35]:


merged_df["is_churned"].value_counts()


# ### Creating further features

# In[36]:


# Fix dot instead of comma if PaidAmount includes any
try:
    merged_df["PaidAmount"] = merged_df["PaidAmount"].str.replace(',','.')
    merged_df["ChargedAmount"] = merged_df["ChargedAmount"].str.replace(',','.')    
except:
    print("Objects are not strings, cannot replace comma with dot.")


# In[37]:


# Convert datatypes
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])
merged_df["CancelDate_y"] = pd.to_datetime(merged_df["CancelDate_y"])
merged_df["ChargedDate"] = pd.to_datetime(merged_df["ChargedDate"])

merged_df["PaidAmount"] = pd.to_numeric(merged_df["PaidAmount"])
merged_df["ChargedAmount"] = pd.to_numeric(merged_df["ChargedAmount"])


# In[38]:


# Fill missing Cancelreasons with zeros
merged_df["Cancelreason"] = merged_df["Cancelreason"].fillna("00000000-0000-0000-0000-000000000000")


# In[39]:


# Months since start
merged_df["months_since_start"] = round((merged_df["PaymentDate"]- merged_df["Startdate"])/np.timedelta64(1, 'M'))

# Months since last payment
merged_df = merged_df.sort_values(by=[ "ContactGUID", "PaymentDate"])
merged_df["months_since_last_payment"] = round(merged_df.groupby(["ContactGUID", "AgreementGUID"])
                                               ["PaymentDate"].diff()/np.timedelta64(1, 'M'))

# Missing payment
merged_df["missing_payment"] = np.where(((merged_df["PaymentStatus"] == 110690003) & (merged_df["PaidAmount"] == 0)) |
                                       ((merged_df["PaymentStatus"] == 110690004) & (merged_df["PaidAmount"] == 0)), True,
                                        False)


# In[40]:


# Add month as feature
merged_df["PaymentMonth"] = merged_df["PaymentDate"].dt.month


# In[41]:


# Months since missing payment feature
grouped = merged_df.groupby("ContactGUID")
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
                result.append(round((row['PaymentDate'] - holder)/np.timedelta64(1, 'M')))
                
merged_df["months_since_missing_payment"] = result


# In[42]:


# Check if calculated monthly values are not negative:
merged_df.loc[merged_df["months_since_start"] < 0, "months_since_start"] = 0
merged_df.loc[merged_df["months_since_last_payment"] < 0, "months_since_last_payment"] = 0
merged_df.loc[merged_df["months_since_missing_payment"] < 0, "months_since_missing_payment"] = 0


# In[43]:


# Predefined columns that should all be in the dataset:
ChannelD_map = {
 110690000: "Betalingsservice",
 110690001: "Giro",
 110690007: "Invoice",
 110690008: "Mobilepay",
 110690009: "Bank",
 110690012: "Card",
 110690013: "MobilepaySubscription"}

PaymentMethod_map = {
 0:         "NoMethod",
 110690000: "Betalingsservice",
 110690001: "Giro",
 110690003: "Sms",
 110690004: "Giro",
 110690006: "Webshop",
 110690007: "Invoice",
 110690008: "Mobilepay",
 110690009: "Bank",
 110690010: "Cash",
 110690012: "Card",
 110690013: "MobilepaySubscription",
 110700002: "SCLspecial",
 110700003: "SCLspecial",
 110700004: "SCLspecial",
 110700007: "SCLspecial",
 110700008: "Bank",
 110700009: "Bank",
 110700013: "SCLspecial"}

PaymentStatus_map = {
 110690000: "Created",
 110690001: "SendForCollection",
 110690002: "PaymentRegistered",
 110690003: "PaymentRejected",
 110690004: "Canceled",
 110690005: "Error",
 110690006: "Refunded"}


# In[44]:


agreement_map = pd.Series(agreement_map_df.agreement_group.values, index=agreement_map_df.agreement_id).to_dict()
create_map = pd.Series(create_map_df.create_group.values, index=create_map_df.create_id).to_dict()
create_map['00000000-0000-0000-0000-000000000000'] = "noReason"

one_hot_list = ["AgreementType", "ChannelD", "CreateReason", "PaymentMethod", "PaymentStatus"]
map_dicts = [agreement_map, ChannelD_map, create_map, PaymentMethod_map, PaymentStatus_map]

for feature, map_dict in zip(one_hot_list, map_dicts):
    merged_df[feature] = merged_df[merged_df[feature].notnull()][feature].apply(
        lambda x: map_dict[x] if x in map_dict else np.nan)    

one_hot_list += ["PaymentMonth"]


# In[45]:


onehot_df = pd.get_dummies(merged_df, columns=one_hot_list)

print("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", flush=True)

# In[46]:


# Removing the .0 from the "ChannelID..." column names because it raises error later with the explainerdashboard
print_flag = True
for n in onehot_df.columns:
    if ".0" in n:
        if print_flag:
            print("Renaming columns with \".0\" in it.")
            print_flag = False
        onehot_df = onehot_df.rename(columns={n:n.replace('.0', '')})


# In[47]:


# Check if there is any column without meaningful data
empty_columns = merged_df.columns[merged_df.isna().all()].tolist()
if empty_columns:
    print("Empty columns:", empty_columns)


# In[48]:


dict_list = [agreement_map, ChannelD_map, create_map, PaymentMethod_map, PaymentStatus_map]
for prefix, map_dict in zip(one_hot_list[:-1], dict_list):
    for feature in [prefix + "_" + a for a in set(map_dict.values())]:
        if feature not in onehot_df.columns:
            print("Adding:\t", feature)
            onehot_df[feature] = 0


# ### New df: 1 row per month per member

# In[49]:


# Convert Postal Code into numerical values
def convert_postcode(postcode):
    if pd.isnull(postcode):
        return postcode
    elif postcode.isnumeric() and len(postcode) == 4:
        return int(postcode)
    else:
        return 0  # postcode is from abroad, naming it as 0 (different from NaN!)


# In[50]:


# Apply to converter function
onehot_df["PostalCode"] = onehot_df.PostalCode.apply(lambda x: convert_postcode(x))


# In[51]:


# Convert string Lottery to numeric (it is a string while the others are boolean):
d = {'True': 1.0, 'False': 0.0}
onehot_df["Lottery"] = onehot_df["Lottery"].map(d)


# In[52]:


# Convert boolean values to numeric
onehot_df["Donation"] = pd.to_numeric(onehot_df["Donation"])
onehot_df["Medlemskab"] = pd.to_numeric(onehot_df["Medlemskab"])
onehot_df["Product"] = pd.to_numeric(onehot_df["Product"])
onehot_df["Lottery"] = pd.to_numeric(onehot_df["Lottery"])


# In[53]:


agg_dict = {
    'Age': 'max',
    'ChargedAmount': 'sum',
    'PostalCode': 'max',
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


# In[54]:


# Group and aggregate
final_df = onehot_df.set_index("PaymentDate").groupby(["ContactGUID", pd.Grouper(freq=pd.offsets.MonthBegin(1))]).agg(agg_dict)

print("11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", flush=True)


# In[55]:


final_df = final_df.reset_index()
final_df.columns = ["_".join(x) for x in final_df.columns.ravel()]


# In[56]:


final_df["is_churned_max"].value_counts()


# In[57]:


print("Size before monthly aggregation:", onehot_df.shape)
print("Size after monthly aggregation:", final_df.shape)


# In[58]:


# Churned within three months
final_df["is_churned_within3m"] = np.where((final_df['CancelDate_y_min'].dt.year <= final_df["PaymentDate_"].dt.year) &
                                   (final_df['CancelDate_y_min'].dt.month <= final_df["PaymentDate_"].dt.month+3), True, False)

# Cummulated total paid amount
final_df["cumulated_paid_amount"] = final_df.groupby(["ContactGUID_"])["PaidAmount_sum"].cumsum()


# In[59]:


final_df["is_churned_within3m"].value_counts()


# In[60]:


# Filling up NaN-s with 0:
final_df["PaidAmount_sum"] = final_df["PaidAmount_sum"].fillna(0) 
final_df["PaymentGUID_count"] = final_df["PaymentGUID_count"].fillna(0)
final_df["cumulated_paid_amount"] = final_df["cumulated_paid_amount"].fillna(0)


# ### Rolling Sums Features

# In[61]:


# Cumulated paid amount in the last three months
final_df["cumulated_amount_3months"] = final_df[["PaidAmount_sum"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
final_df["cumulated_amount_6months"] = final_df[["PaidAmount_sum"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())


# In[62]:


# Number of payments made for the last 3m, last 6m and total
final_df["payment_count_last3m"] = final_df[["PaymentGUID_count"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
final_df["payment_count_last6m"] = final_df[["PaymentGUID_count"]].groupby(
    final_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())

final_df["payment_count_total"] = final_df["PaymentGUID_count"].groupby(
    final_df["ContactGUID_"]).cumsum()


# In[63]:


final_df["is_churned_within3m"] = final_df["is_churned_within3m"].astype(int)


# In[64]:


# Filling up NaN-s with 0:
final_df["cumulated_amount_3months"] = final_df["cumulated_amount_3months"].fillna(0)
final_df["cumulated_amount_6months"] = final_df["cumulated_amount_6months"].fillna(0)
final_df["payment_count_last3m"] = final_df["payment_count_last3m"].fillna(0)
final_df["payment_count_last6m"] = final_df["payment_count_last6m"].fillna(0)


# In[65]:


# Drop features that are not needed/not explore yet
final_df = final_df.drop(["CancelDate_y_min", "is_churned_max"], axis=1)


# ### Check class balance

# In[66]:


class_count = final_df["is_churned_within3m"].value_counts().to_list()
print("Class imbalance:\n\t Not churned in 3 months: {0:.1f}%".format(class_count[0]/sum(class_count)*100))
print("\t Churned in 3 months: {0:.1f}%".format(class_count[1]/sum(class_count)*100))
print("Ratio: {0:.1f}".format(class_count[0]/class_count[1]))
print("Ratio squared: {0:.1f}".format(math.sqrt(class_count[0]/class_count[1])))


# In[67]:


monthly_churn_df = final_df[["is_churned_within3m"]].groupby(final_df["PaymentDate_"]).sum()
monthly_churn_df["total_in_month"] = final_df[["is_churned_within3m"]].groupby(final_df["PaymentDate_"]).size()
monthly_churn_df["not_churned"] = monthly_churn_df["total_in_month"] - monthly_churn_df["is_churned_within3m"]
monthly_churn_df = monthly_churn_df.drop(columns="total_in_month")
monthly_churn_df


# In[68]:


final_df = final_df.set_index(["ContactGUID_", "PaymentDate_"])


# In[69]:


final_df.shape


# ### Data splitting - dates

# The idea is to use the last reasonable three months as labelled data for the **testing set** and the three months prior to that is used as the **validation set**.
# 
# *(We discard the first month if it does not have similar churn numbers as the previous months, right now if it has less than half of the monthly average of all months...)*
# 
# An example:
# - testing month is **May**: its *is_churned_within3m* is labelled with *June, July, August*
# - validation month is **February**: its *is_churned_within3m* is labelled with *March, April, May*
# - the training data covers the rest of the months until *November 2020*
# - the last model has to be fit on an extended merge of the training and the validation data, therefore it includes the two months before the validation.

# In[70]:


churn_last_month = monthly_churn_df.iloc[-1]['is_churned_within3m']
last_date = monthly_churn_df.index.get_level_values(0)[-1]

assert final_df.index.get_level_values(1).max() == last_date


if monthly_churn_df.is_churned_within3m.mean()/2 > churn_last_month:
    test_ends = (last_date.month + 12 - 1) % 12
    print("Test month is shifted 1 month earlier than the last available month due unsufficient amount of data.")
else:
    test_ends = last_date.month
test_month = test_ends - 3
print("Test month is:", test_month)


# In[71]:


val_month = (test_month + 12 - 3) % 12
train_end = (test_month + 12 - 6) % 12

train_dates = pd.date_range(start="1800-1-01", end="2021-" + str(train_end) + "-28")
val_dates = pd.date_range(start="2021-" + str(val_month) + "-01", end="2021-" + str(val_month) + "-28")
test_dates = pd.date_range(start="2021-" + str(test_month) + "-01", end="2021-" + str(test_month) + "-28")

fit_dates = pd.date_range(start="1800-1-01", end="2021-" + str(val_month) + "-28")


# In[72]:


train_df = final_df.loc[(slice(None), train_dates),:]
val_df = final_df.loc[(slice(None), val_dates),:]
test_df = final_df.loc[(slice(None), test_dates),:]
fit_df = final_df.loc[(slice(None), fit_dates),:]


# In[73]:


print("Dates:\nTraining:\t", train_dates.min().date(), "-", train_dates.max().date())
print("Validation:\t", val_dates.min().date(), "-", val_dates.max().date())
print("Testing:\t", test_dates.min().date(), "-", test_dates.max().date())
print("Fitting:\t", fit_dates.min().date(), "-", fit_dates.max().date())

print("\nShapes:\nTotal:\t", final_df.shape[0])
print("Train:\t", train_df.shape[0])
print("Val:\t", val_df.shape[0])
print("Test:\t", test_df.shape[0])
print("Fitting:\t", fit_df.shape[0])
print("Train + val:\t", train_df.shape[0] + val_df.shape[0])


# In[74]:


__X_train = train_df.drop(["is_churned_within3m"], axis=1)
__y_train = train_df["is_churned_within3m"]

__X_val = val_df.drop(["is_churned_within3m"], axis=1)
__y_val = val_df["is_churned_within3m"]

__X_trval = pd.concat([__X_train, __X_val])
__y_trval = pd.concat([__y_train, __y_val])
__X_fit = fit_df.drop(["is_churned_within3m"], axis=1)
__y_fit = fit_df["is_churned_within3m"]

__X_test = test_df.drop(["is_churned_within3m"], axis=1)
__y_test = test_df["is_churned_within3m"]

