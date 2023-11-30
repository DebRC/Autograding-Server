import csv
import sys
import matplotlib.pyplot as plt

num_of_clients = []
response_time = []
throughput_time = []
overall_request_rate = []
overall_error_rate = []
overall_timeout_rate = []
request_rate_sent = []
average_thread = []
average_cpu = []

# open the csv file which contains all the necessary datas
with open(sys.argv[1],'r') as csv_file:
    content = csv.reader(csv_file)
    for line in content:
        if line[0].isdigit() == True:
            num_of_clients.append(int(line[0]))
            response_time.append(float(line[1]))
            throughput_time.append(float(line[2]))
            overall_request_rate.append(float(line[3]))
            overall_error_rate.append(float(line[4]))
            overall_timeout_rate.append(float(line[5]))
            request_rate_sent.append(float(line[6]))
            average_thread.append(float(line[7]))
            average_cpu.append(float(line[8]))


# Clients Vs Throughput plot
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, throughput_time, label = "Clients Vs Throughput")
plt.xlabel("Number of Clients")
plt.ylabel("Throughput (Requests/Sec)")
plt.legend()
# plt.show()
plt.savefig("./observation/throughput.pdf", bbox_inches='tight')
plt.close()



# Clients Vs Response Time plot
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, response_time, label = "Clients Vs Average Response Time")
plt.xlabel("Number of Clients")
plt.ylabel("Average Response Time (Sec)")
plt.legend()
# plt.show()
plt.savefig("./observation/avg_response_time.pdf", bbox_inches='tight')
plt.close()



# client Vs Overall_Request_Rate
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, overall_request_rate, label = "Client Vs Successful Request Rate")
plt.xlabel("Number of Clients")
plt.ylabel("Successful Request Rate(Goodput) (Requests/Sec)")
plt.legend()
# plt.show()
plt.savefig("./observation/overall_request_rate.pdf",bbox_inches='tight')
plt.close()



# client Vs Overall_error_Rate
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, overall_error_rate, label = "Client Vs Overall Error Rate")
plt.xlabel("Number of Clients")
plt.ylabel("Error Rate (Requests/Sec)")
plt.legend()
# plt.show()
plt.savefig("./observation/overall_error_rate.pdf",bbox_inches='tight')
plt.close()



# client Vs Overall_timeout_Rate
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, overall_timeout_rate, label = "Client Vs Overall Time-Out Rate")
plt.xlabel("Number of Clients")
plt.ylabel("Time-Out Rate (Requests/Sec)")
plt.legend()
# plt.show()
plt.savefig("./observation/overall_timeout_rate.pdf",bbox_inches='tight')
plt.close()



# client Vs Overall_Rate_Sent
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, request_rate_sent, label = "Client Vs Request Rate Sent")
plt.xlabel("Number of Clients")
plt.ylabel("Request Rate Sent (Requests/Sec)")
plt.legend()
# plt.show()
plt.savefig("./observation/request_rate_sent.pdf",bbox_inches='tight')
plt.close()



# client Vs Average Threads
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, average_thread, label = "Client Vs Average Threads")
plt.xlabel("Number of Clients")
plt.ylabel("Average Threads")
plt.legend()
# plt.show()
plt.savefig("./observation/average_thread.pdf",bbox_inches='tight')
plt.close()



# client Vs Average CPU
plt.figure(figsize=(8,6))
plt.plot(num_of_clients, average_cpu, label = "Client Vs Average CPU")
plt.xlabel("Number of Clients")
plt.ylabel("Average CPU (Percentage)")
plt.legend()
# plt.show()
plt.savefig("./observation/average_cpu.pdf",bbox_inches='tight')
plt.close()
