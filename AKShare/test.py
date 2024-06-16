import numpy as np
import pandas as pd
from pylab import mpl, plt
import os
import tkinter as tk

plt.style.use('ggplot')  # 设置matplotlib样式
mpl.rcParams['font.family'] = ['serif']  # 指定默认字体

file_path = os.path.abspath(os.path.dirname(__file__))
file_name = file_path + "/source/tr_eikon_eod_data.csv"

f = open(file_name, "r", encoding="utf-8")
data = pd.read_csv(file_name, index_col=0, parse_dates=True)
data.plot(figsize=(10, 12), subplots=True)
plt.close("all")
mpl.use("TkAgg")
plt.show()

import time
time.sleep(10)
