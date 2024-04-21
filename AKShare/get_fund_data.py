# 来源参考：https://mp.weixin.qq.com/s?__biz=MzU1MTM4OTk0MQ==&mid=2247487084&idx=1&sn=b307075af1702d8c1dbda7d9e219a9b5&chksm=fb9351f6cce4d8e0a647c902b25eca89804f8c3169eecd1216f0c5c7d90e64bee900cc7ed2c1&mpshare=1&scene=23&srcid=0309GDbmEHx70KjI19ionXfv&sharer_sharetime=1646819026253&sharer_shareid=a4c6299b7a875e1e5ddbc56b4c71e4dd#rd

import akshare as ak
import pandas as pd
import numpy as np

years = ['2023','2024']

data = pd.DataFrame()
for yr in years:
    df_tmp = ak.fund_portfolio_hold_em(symbol="005827", date = yr)
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
