import akshare as ak
import datetime
import time
import pandas as pd

stock_hot_rank_wc_df = ak.stock_hot_rank_wc(date="20240403")
print(stock_hot_rank_wc_df[['股票简称', '现价','涨跌幅','排名日期']].head(10))