# atm-zl


install gsl library.

./atm filename-corpus filename-authors settings

filename-corpus format :

doc1-length id1:count1 id2:count2 ...

doc2-length id1:count1 id2:count2 ...

...

filename-authors format :

author_id1 author_id2

author_id2

...

settings format :

ETA 1

ALPHA 0.1

TOPIC_NO 50

ETA - smooth word count in the topic.

ALPHA - smooth topic count in the author.

TOPIC_NO - the number of topics.