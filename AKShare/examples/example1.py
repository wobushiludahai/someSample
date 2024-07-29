import akshare as ak

# 获取深圳证券交易所当日成交概览
stock_szse_summary = ak.stock_szse_summary(date="20240726")
print(stock_szse_summary)

# 获取深圳证券交易所地区概览
stock_szse_area_summary = ak.stock_szse_area_summary(date="202406")
print(stock_szse_area_summary)

# 获取深圳证券交易所当月行业概览
stock_sse_summery_df = ak.stock_szse_sector_summary(symbol="当月")
print(stock_sse_summery_df)

