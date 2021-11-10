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
# Rename Membership of it can be found among the column names
if "Membership" in payment_df.columns:
    print("Renaming \"Membership\"")
    payment_df.rename(columns={'Membership': 'Medlemskab'}, inplace=True)

# Fix extra commas in the dataset if it exists
for c in payment_df.columns:
    payment_df = payment_df.rename(columns={c: c.replace(',,,,', '')})
    
if ",,," in str(payment_df["Lottery"][0]):
    print("Converting last column to string.")
    payment_df.Lottery = payment_df.Lottery.astype(str)
    print("Removing extra commas from the last column.")
    payment_df.Lottery = payment_df.Lottery.str.replace(',', '', regex=True)
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

# ### Data Merging

# In[13]:


merged_df = payment_df.merge(agreement_df, on="AgreementGUID", how="left").merge(contact_df,
                                                                                 on="ContactGUID", how = "left")
print("Size after original merge:", merged_df.shape)


# In[14]:


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


# In[15]:


# Remove data with unrelevant cancel reasons
nchurn_contacts = merged_df[merged_df["Cancelreason"].isin(nchurn_reasons)]["ContactGUID"].unique()
merged_df = merged_df[~merged_df["ContactGUID"].isin(nchurn_contacts)]
print("Size after removing unvalid cancel reasons:", merged_df.shape)


# In[16]:


# Remove payments without payment date
merged_df = merged_df[~merged_df["PaymentDate"].isna()]
print("Size after removing payments without date:", merged_df.shape)


# In[17]:


print("Unique person IDs:", merged_df.ContactGUID.nunique())


# In[18]:


# Convert PaymentDate to date type
merged_df["PaymentDate"] = pd.to_datetime(merged_df["PaymentDate"])
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])
merged_df["CancelDate"] = pd.to_datetime(merged_df["CancelDate"])
merged_df["ChargedDate"] = pd.to_datetime(merged_df["ChargedDate"])
merged_df["Startdate"] = pd.to_datetime(merged_df["Startdate"])


# In[19]:


# Change all payment days to the first day of the given month
merged_df["PaymentDate"] = merged_df.PaymentDate.apply(lambda dt: dt.replace(day=1))
merged_df["Startdate"] = merged_df.Startdate.apply(lambda dt: dt.replace(day=1))
merged_df["CancelDate"] = merged_df.CancelDate.apply(lambda dt: dt.replace(day=1))


# In[2]:


print("Number of unique persons who ever cancelled agreement:")
merged_df[merged_df.CancelDate.notnull()].ContactGUID.nunique()


# ### Adding empty rows

# In[21]:


extended_df = merged_df.copy()


# In[22]:


# Add payment minimum and maximum dates to know the range for a given person
extended_df['PaymentDate_min'] = extended_df.groupby('ContactGUID')['PaymentDate'].transform('min')
extended_df['PaymentDate_max'] = extended_df.groupby('ContactGUID')['PaymentDate'].transform('max')


# In[23]:


ids = extended_df.groupby('ContactGUID', group_keys=False).first().index.tolist()
date_range = pd.date_range(
    extended_df["PaymentDate"].min().date(), extended_df["PaymentDate"].max().date(), freq=pd.offsets.MonthBegin(1))


# In[24]:


df = pd.DataFrame(index=ids, columns=date_range)
df['temp']=(' '.join(df.columns.astype(str)))
df = df['temp'].str.split(expand=True)
df["Dates"] = df[df.columns].values.tolist()
df = df[["Dates"]]
df = df.explode("Dates").reset_index().rename(columns = {'index':'ContactGUID', 'Dates':'PaymentDate'})
df["PaymentDate"] = pd.to_datetime(df["PaymentDate"])


# In[25]:


extended_df = extended_df.merge(df, on=["PaymentDate", "ContactGUID"],
                          how='outer', indicator=True).reset_index().sort_values(by=["ContactGUID", "PaymentDate"])


# In[26]:


# Is the payment associated with an agreement
extended_df["payment_on_agreement"] = np.where(extended_df["AgreementGUID"].isna(), False, True)


# In[27]:


# Change payment_on_agreement flag to True if the person has ever had an agreement
extended_df['payment_on_agreement'] = extended_df.groupby('ContactGUID')['payment_on_agreement'].transform('max')


# In[28]:


# Copy mimimum payment dates to the new cells
extended_df["PaymentDate_min"] = extended_df.sort_values(
    ['ContactGUID','PaymentDate_min'],ascending=True).groupby('ContactGUID').PaymentDate_min.ffill()
extended_df["PaymentDate_max"] = extended_df.sort_values(
    ['ContactGUID','PaymentDate_max'],ascending=True).groupby('ContactGUID').PaymentDate_max.ffill()


# In[29]:


# Remove erlier rows than the first payment of the person
payments_not_earlier = np.where(extended_df['PaymentDate'] >= extended_df["PaymentDate_min"], True, False)
extended_df = extended_df[payments_not_earlier]


# In[30]:


# Remove later rows than the last payment of the person if there is no agreement
# (we cannot know if the person has already cancelled the agreement or not...)
payment_after_without_agreement = np.where((extended_df['PaymentDate'] > extended_df["PaymentDate_max"])
                                            & ~extended_df["payment_on_agreement"], True, False)
extended_df = extended_df[~payment_after_without_agreement]


# In[31]:


print("Size with dummy rows only inside the range:", extended_df.shape)


# ### New df: when was the first churned agreement for each member

# In[32]:


# Get the date of the first churned agreement per person
churned_df = extended_df[extended_df["CancelDate"].isna() == False].groupby(
    "ContactGUID", as_index = False)["CancelDate"].min()


# In[33]:


extended_df = extended_df.merge(churned_df, on="ContactGUID", how="left")


# ### Creating features

# In[34]:


feature_df = extended_df.copy()


# In[35]:


# Fix dot instead of comma if PaidAmount includes any
try:
    feature_df["PaidAmount"] = feature_df["PaidAmount"].str.replace(',','.')
    feature_df["ChargedAmount"] = feature_df["ChargedAmount"].str.replace(',','.')    
except:
    print("Objects are not strings, cannot replace comma with dot.")


# In[36]:


# Convert datatypes
feature_df["Startdate"] = pd.to_datetime(feature_df["Startdate"])
feature_df["CancelDate_y"] = pd.to_datetime(feature_df["CancelDate_y"])
feature_df["ChargedDate"] = pd.to_datetime(feature_df["ChargedDate"])

feature_df["PaidAmount"] = pd.to_numeric(feature_df["PaidAmount"])
feature_df["ChargedAmount"] = pd.to_numeric(feature_df["ChargedAmount"])


# In[37]:


# Fill missing Cancelreasons with zeros
feature_df["Cancelreason"] = feature_df["Cancelreason"].fillna("00000000-0000-0000-0000-000000000000")


# In[38]:


# Churn flag for those who have a cancel date with not older date than the payment date OR those who have future cancel registered. 
feature_df['is_churned'] = False
feature_df['is_churned'] = np.where(((feature_df['CancelDate_y'].dt.month <= feature_df["PaymentDate"].dt.month) &
                                   (feature_df['CancelDate_y'].dt.year <= feature_df["PaymentDate"].dt.year)) | 
                                    feature_df.FutureCancel == True, True, False)


# In[39]:


# Remove payments after churn date
feature_df = feature_df[~feature_df["index"].isin(
    feature_df[feature_df["PaymentDate"] > feature_df["CancelDate_y"]]["index"])]


# In[40]:


print("Size after removing payments after a churn has occured:", feature_df.shape)


# In[41]:


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


# In[42]:


feature_df.missing_payment.value_counts()


# In[43]:


# Add month as feature
feature_df["PaymentMonth"] = feature_df["PaymentDate"].dt.month


# In[44]:


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


# In[45]:


feature_df["is_churned"].value_counts()


# In[46]:


# Check if there is any column without meaningful data
feature_df.columns[feature_df.isna().all()].tolist()


# In[47]:


# Predefined columns that should all be in the dataset:
ChannelD_all = [
 'ChannelD_110690000',
 'ChannelD_110690001',
 'ChannelD_110690007',
 'ChannelD_110690008',
 'ChannelD_110690009',
 'ChannelD_110690012',
 'ChannelD_110690013']

PaymentMethod_all = [
 'PaymentMethod_0',
 'PaymentMethod_110690000',
 'PaymentMethod_110690001',
 'PaymentMethod_110690003',
 'PaymentMethod_110690004',
 'PaymentMethod_110690006',
 'PaymentMethod_110690007',
 'PaymentMethod_110690008',
 'PaymentMethod_110690009',
 'PaymentMethod_110690010',
 'PaymentMethod_110690012',
 'PaymentMethod_110690013',
 'PaymentMethod_110700002',
 'PaymentMethod_110700003',
 'PaymentMethod_110700004',
 'PaymentMethod_110700007',
 'PaymentMethod_110700008',
 'PaymentMethod_110700009',
 'PaymentMethod_110700013']

PaymentStatus_all = [
 'PaymentStatus_110690001',
 'PaymentStatus_110690002',
 'PaymentStatus_110690003',
 'PaymentStatus_110690004',
 'PaymentStatus_110690005',
 'PaymentStatus_110690006',
 'PaymentStatus_110690007']


# In[48]:


# Most frequent Agreement types:

GFNG_Agreements = [
    'b53c675d-db04-e811-8116-5065f38aea11', #: 342060,
 'a33c675d-db04-e811-8116-5065f38aea11', #: 78212,
 '0b3d675d-db04-e811-8116-5065f38aea11', #: 73703,
 'b73c675d-db04-e811-8116-5065f38aea11', #: 37044,
 'a93c675d-db04-e811-8116-5065f38aea11', #: 8117,
 'a13c675d-db04-e811-8116-5065f38aea11', #: 6823,
 'c53c675d-db04-e811-8116-5065f38aea11', #: 3405,
 'c73c675d-db04-e811-8116-5065f38aea11', #: 3397,
 'c93c675d-db04-e811-8116-5065f38aea11', #: 2874,
 'a4a0d12f-009c-e911-a962-000d3ab6488a', #: 2689,
]

SCL_Agreements = [
 '5356f703-f17f-ea11-a811-000d3ada453a', #: 280387,
 '6e142508-f17f-ea11-a811-000d3aba7e84', #: 30072,
 '5456f703-f17f-ea11-a811-000d3ada453a', #: 19542,
 '5156f703-f17f-ea11-a811-000d3ada453a', #: 18615,
 '4e56f703-f17f-ea11-a811-000d3ada453a', #: 9185,
#  '5256f703-f17f-ea11-a811-000d3ada453a': 822,
#  '5556f703-f17f-ea11-a811-000d3ada453a': 361,
#  '24a6035f-8322-eb11-a813-000d3aba7e84': 207,
#  '9256f703-f17f-ea11-a811-000d3ada453a': 186,
#  '9356f703-f17f-ea11-a811-000d3ada453a': 174,
]

# Most frequent Create Reason types:

GFNG_CreateReasons = [
 'f4342465-c87b-e911-a95c-000d3ab6413d', #: 419362,
 '6f25342f-c839-e911-a959-000d3ab64750', #: 56609,
 'aba12d29-c839-e911-a959-000d3ab64750', #: 37713,
 '188d8de0-a4b7-ea11-a812-000d3aba8599', #: 29533,
 'daadfb5a-c739-e911-a959-000d3ab64750', #: 6932,
 'd5ed5061-c739-e911-a959-000d3ab64750', #: 2220,
 '7f16ff01-923a-e911-a959-000d3ab64750', #: 1664,
 '6c4627c0-49a0-ea11-a812-000d3ad9183c', #: 1652,
 '100b0ee3-9771-e911-a95f-000d3ab6488a', #: 1170,
 'a2cf8a50-394f-eb11-a812-000d3aba8599', #: 1140,
]

SCL_CreateReasons = [
 '2c0a05e3-607e-ea11-a811-000d3aba7e84', #: 64085,
 '540a05e3-607e-ea11-a811-000d3aba7e84', #: 48404,
 '610a05e3-607e-ea11-a811-000d3aba7e84', #: 38777,
 '120a05e3-607e-ea11-a811-000d3aba7e84', #: 19030,
 '411b19de-607e-ea11-a811-000d3ada453a', #: 18233,
 'de0905e3-607e-ea11-a811-000d3aba7e84', #: 15467,
 '521b19de-607e-ea11-a811-000d3ada453a', #: 15407,
 '6b1b19de-607e-ea11-a811-000d3ada453a', #: 14280,
 '110a05e3-607e-ea11-a811-000d3aba7e84', #: 13965,
 '180a05e3-607e-ea11-a811-000d3aba7e84', #: 9271,
]

used_agreements = GFNG_Agreements + SCL_Agreements
used_createreasons = GFNG_CreateReasons + SCL_CreateReasons

Agreements_all = ["AgreementType_" + a for a in used_agreements]
CreateReasons_all = ["CreateReason_" + a for a in used_createreasons]


# In[49]:


# Replace rare types with "other"
feature_df.AgreementType = feature_df.AgreementType.apply(lambda x: x if x in used_agreements else "other")
feature_df.CreateReason = feature_df.CreateReason.apply(lambda x: x if x in used_createreasons else "other")


# In[50]:


one_hot_list = ["PaymentMethod", "PaymentStatus", "ChannelD", "AgreementType", "CreateReason", "PaymentMonth"]
onehot_df = pd.get_dummies(feature_df, columns=one_hot_list)


# In[51]:


# Removing the .0 from the "ChannelID..." column names because it raises error later with the explainerdashboard
print_flag = True
for n in onehot_df.columns:
    if ".0" in n:
        if print_flag:
            print("Renaming columns with \".0\" in it.")
            print_flag = False
        onehot_df = onehot_df.rename(columns={n:n.replace('.0', '')})


# In[52]:


for feature in ChannelD_all + PaymentMethod_all + PaymentStatus_all + Agreements_all + CreateReasons_all:
    if feature not in onehot_df.columns:
        onehot_df[feature] = 0


# ### New df: 1 row per month per member

# In[53]:


# Convert Postal Code into numerical values
def convert_postcode(postcode):
    if pd.isnull(postcode):
        return postcode
    elif postcode.isnumeric() and len(postcode) == 4:
        return int(postcode)
    else:
        return 0  # postcode is from abroad, naming it as 0 (different from NaN!)


# In[54]:


# Apply to converter function
onehot_df["PostalCode"] = onehot_df.PostalCode.apply(lambda x: convert_postcode(x))


# In[55]:


# Convert string Lottery to numeric:
d = {'True': 1.0, 'False': 0.0}
onehot_df["Lottery"] = onehot_df["Lottery"].map(d)


# In[56]:


# Convert sum values to numeric (there are some boolean for some reason)
onehot_df["Donation"] = pd.to_numeric(onehot_df["Donation"])
onehot_df["Medlemskab"] = pd.to_numeric(onehot_df["Medlemskab"])
onehot_df["Product"] = pd.to_numeric(onehot_df["Product"])


# In[57]:


agg_dict = {
#     'ChargedDate': 'min',
    'ChargedAmount': 'sum',
#     'Startdate': 'min',
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


# In[58]:


# Group and aggregate
agg_df = onehot_df.set_index("PaymentDate").groupby(["ContactGUID", pd.Grouper(freq=pd.offsets.MonthBegin(1))]).agg(agg_dict)


# In[59]:


agg_df["is_churned"].value_counts()


# In[60]:


agg_df = agg_df.reset_index()
agg_df.columns = ["_".join(x) for x in agg_df.columns.ravel()]


# In[61]:


print("Size before aggregation:", onehot_df.shape)
print("Size after aggregation:", agg_df.shape)


# In[62]:


# Churned within three months
agg_df["is_churned_within3m"] = np.where((agg_df['CancelDate_y_min'].dt.year <= agg_df["PaymentDate_"].dt.year) &
                                   (agg_df['CancelDate_y_min'].dt.month <= agg_df["PaymentDate_"].dt.month+3), True, False)

# Cummulated total paid amount
agg_df["cumulated_paid_amount"] = agg_df.groupby(["ContactGUID_"])["PaidAmount_sum"].cumsum()


# In[63]:


agg_df["is_churned_within3m"].value_counts()


# In[64]:


# Filling up NaN-s with 0:
agg_df["PaidAmount_sum"] = agg_df["PaidAmount_sum"].fillna(0) 
agg_df["PaymentGUID_count"] = agg_df["PaymentGUID_count"].fillna(0)
agg_df["cumulated_paid_amount"] = agg_df["cumulated_paid_amount"].fillna(0)


# ### Rolling Sums Features

# In[65]:


# Cumulated paid amount in the last three months
roll_df = agg_df.copy()

roll_df["cumulated_amount_3months"] = roll_df[["PaidAmount_sum"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
roll_df["cumulated_amount_6months"] = roll_df[["PaidAmount_sum"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())


# In[66]:


# Number of payments made for the last 3m, last 6m and total
roll_df["payment_count_last3m"] = roll_df[["PaymentGUID_count"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(3).sum())
roll_df["payment_count_last6m"] = roll_df[["PaymentGUID_count"]].groupby(
    roll_df["ContactGUID_"]).apply(lambda g: g.rolling(6).sum())

roll_df["payment_count_total"] = roll_df["PaymentGUID_count"].groupby(
    roll_df["ContactGUID_"]).cumsum()


# In[67]:


roll_df["is_churned_within3m"] = roll_df["is_churned_within3m"].astype(int)


# In[68]:


# Filling up NaN-s with 0:
roll_df["cumulated_amount_3months"] = roll_df["cumulated_amount_3months"].fillna(0)
roll_df["cumulated_amount_6months"] = roll_df["cumulated_amount_6months"].fillna(0)
roll_df["payment_count_last3m"] = roll_df["payment_count_last3m"].fillna(0)
roll_df["payment_count_last6m"] = roll_df["payment_count_last6m"].fillna(0)


# In[69]:


# Drop features that are not needed/not explore yet
final_df = roll_df.drop(["CancelDate_y_min", "is_churned_max"], axis=1)

class_count = final_df["is_churned_within3m"].value_counts().to_list()
print("Class imbalance:\n\t Not churned in 3 months:", class_count[0]/sum(class_count)*100)

monthly_churn_df = final_df[["is_churned_within3m"]].groupby(final_df["PaymentDate_"]).sum()
monthly_churn_df["total_in_month"] = final_df[["is_churned_within3m"]].groupby(final_df["PaymentDate_"]).size()
monthly_churn_df["not_churned"] = monthly_churn_df["total_in_month"] - monthly_churn_df["is_churned_within3m"]
monthly_churn_df = monthly_churn_df.drop(columns="total_in_month")

final_df = final_df.set_index(["ContactGUID_", "PaymentDate_"])

print(final_df.shape)
print(final_df.columns)

__result = final_df





