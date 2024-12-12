all : consumer producer


consumer : consumer.cpp
	g++ -o consumer consumer.cpp

producer : producer.cpp
	g++ -o producer producer.cpp

delete: delete.cpp
	g++ delete.cpp -o delete && ./delete

clean :
	rm -f consumer producer delete