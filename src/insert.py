import pandas as pd
import MySQLdb
import sys

connection = MySQLdb.connect(db="contest",user="root",passwd="password")
connection.set_character_set('utf8')
cursor = connection.cursor()
train = pd.read_csv('../data/train.csv').fillna(' ')
for _, row in train.iloc[:10000].iterrows():
    # CREATE TABLE train(comment_text text, superviced int);
    sql = "INSERT INTO train(comment_text, superviced) VALUES(%s, %s)"
    try:
        cursor.execute(sql, (row["comment_text"].replace("\n", ""), row["toxic"]))
        print("ok")
    except Exception:
        continue
    
connection.commit()