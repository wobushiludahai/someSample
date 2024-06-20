import akshare as ak

stock_sse_summery_df = ak.stock_sse_summary()
print(stock_sse_summery_df)



stock_sse_summery_df = ak.stock_szse_sector_summary(symbol="当月")
print(stock_sse_summery_df)

