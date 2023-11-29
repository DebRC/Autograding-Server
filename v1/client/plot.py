import csv
import sys
import matplotlib.pyplot as plt

# Lists used to plot the graphs
num_of_clients = []
response_time = []
throughput_time = []

# opening the given input csv file and taking the values for the plot
with open(sys.argv[1],'r') as csv_file:
    content = csv.reader(csv_file)
    for line in content:
        if line[0].isdigit() == True:
            num_of_clients.append(int(line[0]))
            response_time.append(float(line[1]))
            throughput_time.append(float(line[2]))


# Clients Vs Throughput plot
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, throughput_time, label = "Clients Vs Throughput")
plt.xlabel("Number of Clients")
plt.ylabel("Throughput (Requests/Sec)")
plt.legend()
# plt.show()
plt.savefig("throughput.pdf", bbox_inches='tight')
plt.close()

# Clients Vs Response Time plot
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, response_time, label = "Clients Vs Average Response Time")
plt.xlabel("Number of Clients")
plt.ylabel("Average Response Time (Sec)")
plt.legend()
# plt.show()
plt.savefig("avg_response_time.pdf", bbox_inches='tight')
plt.close()

