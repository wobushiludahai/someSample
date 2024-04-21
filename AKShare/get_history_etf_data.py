import akshare as ak
import pandas as pd
import numpy as np
import plotly.express as px
import plotly.graph_objects as go

etf = ak.fund_etf_category_sina(symbol="ETF基金")
# etf.to_csv("sina_etf_list.csv", encoding='utf-8-sig')
# print(etf)
hkmi = ak.fund_etf_hist_sina(symbol="sh513060")
# print(hkmi)
fig = px.line(hkmi, x="date", y="close", title='恒生医疗ETF收盘价')

fig.add_trace(go.Scatter(x=[hkmi['date'].iloc[-1]],
                         y=[hkmi['close'].iloc[-1]],
                         text=[hkmi['date'].iloc[-1]],
                         mode='markers+text',
                         marker=dict(color='red', size=10),
                         textfont=dict(color='green', size=10),
                         textposition='top left',
                         showlegend=False))

fig.add_trace(go.Scatter(x=[hkmi['date'].iloc[-1]],
                         y=[hkmi['close'].iloc[-1]],
                         text=[hkmi['close'].iloc[-1]],
                         mode='markers+text',
                         marker=dict(color='red', size=10),
                         textfont=dict(color='green', size=10),
                         textposition='bottom center',
                         showlegend=False))

fig.show()