echo "Running 1..."
./pm 1 >> output1.txt
echo "Running 2..."
./pm 2 >> output2.txt
echo "Running 3 10..."
./pm 3 10 >> output3_10.txt
echo "Running 3 20..."
./pm 3 20 >> output3_20.txt
echo "Running 3 50..."
./pm 3 50 >> output3_50.txt
echo "Running 3 250..."
./pm 3 250 >> output3_250.txt
echo "Running 3 500..."
./pm 3 500 >> output3_500.txt
echo "Done."