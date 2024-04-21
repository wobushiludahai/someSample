import akshare as ak
import pandas as pd
import numpy as np

years = ['2023','2024']

data = pd.DataFrame()
for yr in years:
    df_tmp = ak.fund_portfolio_hold_em(symbol="005827", date = yr)
    # print(df_tmp)
    data = pd.concat([data,df_tmp], ignore_index=True)

data['季度'] = data['季度'].apply(lambda x:x[:6])
data['季度'] = data['季度'].str.replace('年','Q')
data['占净值比例'] = pd.to_numeric(data['占净值比例'])
print(data)

s1 = '2023Q4'
s2 = '2024Q1'
s1_share = s1+'持股数'
s2_share = s2+'持股数'
s1_value = s1+'持仓市值'
s2_value = s2+'持仓市值'
s1_ratio = s1+'持仓比例'
s2_ratio = s2+'持仓比例'

df1 =data[data['季度']==s1]
df1 = df1[['股票代码', '股票名称','持股数','持仓市值','占净值比例']]
df1 = df1.rename(columns={'持股数':s1_share,'持仓市值':s1_value,'占净值比例':s1_ratio})
df2 =data[data['季度']==s2]
df2 = df2[['股票代码', '股票名称','持股数','持仓市值','占净值比例']]
df2 = df2.rename(columns={'持股数':s2_share,'持仓市值':s2_value,'占净值比例':s2_ratio})
print(df1)
print(df2)
# print(data)

# print(f'pandas version:{pd.__version__}')
# print(f'akshare version: {ak.__version__}')