import time

def get_today_date():
    today = time.strftime("%Y%m%d", time.localtime())
    return today