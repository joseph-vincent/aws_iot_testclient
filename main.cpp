#include <aws/crt/Api.h>
#include <aws/crt/StlAllocator.h>
#include <aws/crt/auth/Credentials.h>
#include <aws/crt/io/TlsOptions.h>

#include <aws/iot/MqttClient.h>

#include <algorithm>
#include <aws/crt/UUID.h>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include <fstream>

#include <sstream>
#include <string>

//#include <snappy.h>
//#include "lz4.h"
//#include "zlib.h"
#include <queue>

#include <ctime>


using namespace Aws::Crt;

struct MessageData
{
	std::string buffer;
	std::string topic;
};
std::queue<MessageData> messageQueue;
pthread_cond_t  cond_queue = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
unsigned int sequenceNumber =0;

static void s_printHelp()
{
    fprintf(stdout, "Usage:\n");
    fprintf(
        stdout,
        "agentCpp --endpoint <endpoint> --cert <path to cert>"
        " --key <path to key> --topic <topic> --ca_file <path to custom ca>"
	" [--clientid <MQTT client ID to use>] [--compress <zlib/lz4/snappy>] [--thingname <name of the thing>]\n\n");
    fprintf(stdout, "endpoint: the endpoint of the mqtt server not including a port\n");
    fprintf(
        stdout,
        "cert: path to your client certificate in PEM format. If this is not set you must specify use_websocket\n");
    fprintf(stdout, "key: path to your key in PEM format. If this is not set you must specify use_websocket\n");
    fprintf(stdout, "topic: topic to subscribe to. (optional)\n");
    fprintf(stdout, "statustopic: topic to publish status. (optional)\n");
    fprintf(stdout, "clientid: client id to use \n");
    fprintf(stdout, "thingname: thing name\n");
    fprintf(stdout, "compress: compress payload or not (default : false)\n");
    fprintf(
        stdout,
        "ca_file: Optional, if the mqtt server uses a certificate that's not already"
        " in your trust store, set this.\n");
    fprintf(stdout, "\tIt's the path to a CA file in PEM format\n");
}

bool s_cmdOptionExists(char **begin, char **end, const String &option)
{
    return std::find(begin, end, option) != end;
}

char *s_getCmdOption(char **begin, char **end, const String &option)
{
	char **itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

/*
int compressSnappy (char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{
  std::string compressed;
  snappy::Compress( inBuffer, inBufferLen, &compressed );
  memcpy(outBuffer,compressed.data(),compressed.length());
  return compressed.length();
}

int decompressSnappy (char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{
  std::string uncompressed;
  snappy::Uncompress( inBuffer, inBufferLen, &uncompressed );
  memcpy(outBuffer,uncompressed.data(),uncompressed.length());
  return uncompressed.length();
}

int compressLZ4 (char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{

	return  LZ4_compress((const char *)(inBuffer), outBuffer, inBufferLen);
}

int decompressLZ4 (char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{
	//LZ4_uncompress(inBuffer, outBuffer, inBufferLen);
	return LZ4_decompress_safe(inBuffer,outBuffer, inBufferLen, outBufferLen);

	//return strlen(outBuffer);
}
int compressZLIB (char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{
	z_stream defstream;
	defstream.zalloc = Z_NULL;
	defstream.zfree = Z_NULL;
	defstream.opaque = Z_NULL;
	// setup buffers for input and compressed output
	defstream.avail_in = (uInt)inBufferLen; // size of input, string + terminator
	defstream.next_in = (Bytef *)inBuffer; // input char array
	defstream.avail_out = (uInt)outBufferLen; // size of output
	defstream.next_out = (Bytef *)outBuffer; // output char array

	// the actual compression work.
	deflateInit(&defstream, Z_BEST_COMPRESSION);
	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);


	return defstream.total_out;
}

int decompressZLIB (char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{

	// inflate b into c
	// zlib struct
	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;
	// setup "compressed" as the input and "uncompressed" as the compressed output
	infstream.avail_in = (uInt)inBufferLen; // size of input
	infstream.next_in = (Bytef *)inBuffer; // input char array
	infstream.avail_out = (uInt)outBufferLen; // size of output
	infstream.next_out = (Bytef *)outBuffer; // output char array

	// the actual DE-compression work.
	inflateInit(&infstream);
	inflate(&infstream, Z_NO_FLUSH);
	inflateEnd(&infstream);
	return infstream.total_out;
}
*/

int compress (String type, char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{
	//if (type == "zlib")
	//{
	//	return compressZLIB (inBuffer, inBufferLen, outBuffer, outBufferLen);
	//}
	//else if (type == "lz4")
	//{
	//	return compressLZ4 (inBuffer, inBufferLen, outBuffer, outBufferLen);
	//}
	//else if (type == "snappy")
	//{
	//	return compressSnappy (inBuffer, inBufferLen, outBuffer, outBufferLen);
	//}
	//else
	{
		memcpy(outBuffer,inBuffer,inBufferLen);
		return inBufferLen;
	}
}
int decompress (String type, char * inBuffer, unsigned int inBufferLen, char * outBuffer, unsigned int outBufferLen)
{
	//if (type == "zlib")
	//{
	//	return decompressZLIB (inBuffer, inBufferLen, outBuffer, outBufferLen);
	//}
	//else if (type == "lz4")
	//{
	//	return decompressLZ4 (inBuffer, inBufferLen, outBuffer, outBufferLen);
	//}
	//else if (type == "snappy")
	//{
	//	return decompressSnappy (inBuffer, inBufferLen, outBuffer, outBufferLen);
	//}
	//else
	{
		memcpy(outBuffer,inBuffer,inBufferLen);
		return inBufferLen;
	}
}

int sendMessage( char *topicName, char* payload, unsigned int payloadlen)                                                      
{                                                                                         
	if((nullptr != payload) && (nullptr != topicName))                                    
	{                                                                                     
		fprintf(stdout, "Topic Name             :%s\n",topicName);
		fprintf(stdout, "Message Payload Length :%d\n",payloadlen);     

		pthread_mutex_lock(&lock);
        {
            char *buffer = new char [payloadlen+1];
            memset(buffer,0,payloadlen+1);
            memcpy(buffer,payload,payloadlen);
    		fprintf(stdout, "Message Payload Data   :%s\n",buffer);

            MessageData messageData = {buffer,topicName};
            
            messageQueue.push(messageData);
            pthread_cond_signal(&cond_queue);

            sequenceNumber++;
        }
        pthread_mutex_unlock(&lock);
	}                                                                                     
	return 1;                                                                             
}


int main(int argc, char *argv[])
{

	/************************ Setup the Lib ****************************/
	/*
	 * Do the global initialization for the API.
	 */
	ApiHandle apiHandle;
    apiHandle.InitializeLogging( Aws::Crt::LogLevel::Trace,stdout);


	String endpoint;
	String certificatePath;
	String keyPath;
	String caFile;
	String subscribeTopic("topic_1");
	String publishTopic("test/agent/status");
	String clientId(String("basicPubSub"));
	String thingName(String("cppAgent"));
	String signingRegion;
	String compressType = "none";

	/*********************** Parse Arguments ***************************/
	fprintf(
			stdout,
			"  Parsing Arguments \n");
	if (!s_cmdOptionExists(argv, argv + argc, "--endpoint"))
	{
		s_printHelp();
		return 1;
	}

	endpoint = s_getCmdOption(argv, argv + argc, "--endpoint");

	if (s_cmdOptionExists(argv, argv + argc, "--key"))
	{
		keyPath = s_getCmdOption(argv, argv + argc, "--key");
	}

	if (s_cmdOptionExists(argv, argv + argc, "--cert"))
	{
		certificatePath = s_getCmdOption(argv, argv + argc, "--cert");
	}

	if (keyPath.empty() != certificatePath.empty())
	{
		fprintf(stdout, "Using mtls (cert and key) requires both the certificate and the private key\n");
		s_printHelp();
		return 1;
	}
	if (s_getCmdOption(argv, argv + argc, "--topic"))
	{
		subscribeTopic = s_getCmdOption(argv, argv + argc, "--topic");
	}
	if (s_getCmdOption(argv, argv + argc, "--statustopic"))
	{
		publishTopic = s_getCmdOption(argv, argv + argc, "--statustopic");
	}
	if (s_cmdOptionExists(argv, argv + argc, "--ca_file"))
	{
		caFile = s_getCmdOption(argv, argv + argc, "--ca_file");
	}
	if (s_cmdOptionExists(argv, argv + argc, "--clientid"))
	{
		clientId = s_getCmdOption(argv, argv + argc, "--clientid");
	}
	if (s_cmdOptionExists(argv, argv + argc, "--thingname"))
	{
		thingName = s_getCmdOption(argv, argv + argc, "--thingname");
	}

	if (s_cmdOptionExists(argv, argv + argc, "--compress"))
	{
		compressType = s_getCmdOption(argv, argv + argc, "--compress");
		if ((compressType != "none") && (compressType != "zlib") && (compressType != "lz4") && (compressType != "snappy"))
		{
			fprintf( stderr,
					"Invalid Compression Type %s\n", compressType.c_str());
			exit(-1);
		}
	}


	/********************** Now Setup an Mqtt Client ******************/
	/*
	 * You need an event loop group to process IO events.
	 * If you only have a few connections, 1 thread is ideal
	 */
	Io::EventLoopGroup eventLoopGroup(1);
	if (!eventLoopGroup)
	{
		fprintf(
				stderr, "Event Loop Group Creation failed with error %s\n", ErrorDebugString(eventLoopGroup.LastError()));
		exit(-1);
	}

	Aws::Crt::Io::DefaultHostResolver defaultHostResolver(eventLoopGroup, 1, 5);
	Io::ClientBootstrap bootstrap(eventLoopGroup, defaultHostResolver);

	if (!bootstrap)
	{
		fprintf(stderr, "ClientBootstrap failed with error %s\n", ErrorDebugString(bootstrap.LastError()));
		exit(-1);
	}

	Aws::Iot::MqttClientConnectionConfigBuilder builder;

	if (!certificatePath.empty() && !keyPath.empty())
	{
		fprintf(stdout, "Creating Builder for Client Config for MQTT Client --\n");
		builder = Aws::Iot::MqttClientConnectionConfigBuilder(certificatePath.c_str(), keyPath.c_str());
	}
	else
	{
		s_printHelp();
	}

	if (!caFile.empty())
	{
		builder.WithCertificateAuthority(caFile.c_str());
	}

	builder.WithEndpoint(endpoint);

	fprintf(stdout, "Creating Client Config for MQTT Client --\n");
	auto clientConfig = builder.Build();

	if (!clientConfig)
	{
		fprintf(
				stderr,
				"Client Configuration initialization failed with error %s\n",
				ErrorDebugString(clientConfig.LastError()));
		exit(-1);
	}

	fprintf(stdout, "Creating MQTT Client --\n");
	Aws::Iot::MqttClient mqttClient(bootstrap);
	/*
	 * Since no exceptions are used, always check the bool operator
	 * when an error could have occurred.
	 */
	if (!mqttClient)
	{
		fprintf(stderr, "MQTT Client Creation failed with error %s\n", ErrorDebugString(mqttClient.LastError()));
		exit(-1);
	}

	/*
	 * Now create a connection object. Note: This type is move only
	 * and its underlying memory is managed by the client.
	 */
	auto connection = mqttClient.NewConnection(clientConfig);

	if (!connection)
	{
		fprintf(stderr, "MQTT Connection Creation failed with error %s\n", ErrorDebugString(mqttClient.LastError()));
		exit(-1);
	}

	/*
	 * In a real world application you probably don't want to enforce synchronous behavior
	 * but this is a sample console application, so we'll just do that with a condition variable.
	 */
	std::promise<bool> connectionCompletedPromise;
	std::promise<void> connectionClosedPromise;

	/*
	 * This will execute when an mqtt connect has completed or failed.
	 */
	auto onConnectionCompleted = [&](Mqtt::MqttConnection &, int errorCode, Mqtt::ReturnCode returnCode, bool) {
		if (errorCode)
		{
			fprintf(stdout, "Connection failed with error %s\n", ErrorDebugString(errorCode));
			connectionCompletedPromise.set_value(false);
		}
		else
		{
			if (returnCode != AWS_MQTT_CONNECT_ACCEPTED)
			{
				fprintf(stdout, "Connection failed with mqtt return code %d\n", (int)returnCode);
				connectionCompletedPromise.set_value(false);
			}
			else
			{
				fprintf(stdout, "Connection successfull.\n");
				connectionCompletedPromise.set_value(true);
			}
		}
	};

	auto onInterrupted = [&](Mqtt::MqttConnection &, int error) {
		//fprintf(stdout, "Connection interrupted with error %s\n", ErrorDebugString(error));
	};

	auto onResumed = [&](Mqtt::MqttConnection &, Mqtt::ReturnCode, bool) { };//fprintf(stdout, "Connection resumed\n"); };

	/*
	 * Invoked when a disconnect message has completed.
	 */
	auto onDisconnect = [&](Mqtt::MqttConnection &) {
		{
			fprintf(stdout, "Disconnect completed\n");
			connectionClosedPromise.set_value();
		}
	};

	connection->OnConnectionCompleted = std::move(onConnectionCompleted);
	connection->OnDisconnect = std::move(onDisconnect);
	connection->OnConnectionInterrupted = std::move(onInterrupted);
	connection->OnConnectionResumed = std::move(onResumed);

	connection->SetOnMessageHandler([](Mqtt::MqttConnection &,
				const String &topic,
				const ByteBuf &payload,
				bool /*dup*/,
				Mqtt::QOS /*qos*/,
				bool /*retain*/) {
			fprintf(stdout, "Generic Publish received on topic %s, payload:\n", topic.c_str());
			fwrite(payload.buffer, 1, payload.len, stdout);
			fprintf(stdout, "\n");
			});

	/*
	 * Actually perform the connect dance.
	 * This will use default ping behavior of 1 hour and 3 second timeouts.
	 * If you want different behavior, those arguments go into slots 3 & 4.
	 */
	fprintf(stdout, "Connecting to AWS MQTT with client ID %s...\n",clientId.c_str());
	if (!connection->Connect(clientId.c_str(), false, 1000))
	{
		fprintf(stdout, "MQTT Connection failed with error %s\n", ErrorDebugString(connection->LastError()));
		exit(-1);
	}

	if (connectionCompletedPromise.get_future().get())
	{
		/*
		 * This is invoked upon the receipt of a Publish on a subscribed topic.
		 */
		auto onMessage = [&](Mqtt::MqttConnection &,
				const String &topic,
				const ByteBuf &byteBuf,
				bool /*dup*/,
				Mqtt::QOS /*qos*/,
				bool /*retain*/) {
			fprintf(stdout, "Publish received on topic %s\n", topic.c_str());
			fprintf(stdout, "\n Message:\n");
			fwrite(byteBuf.buffer, 1, byteBuf.len, stdout);
			fprintf(stdout, "\n");
		};

		/*
		 * Subscribe for incoming publish messages on topic.
		 */
		std::promise<void> subscribeFinishedPromise;
		auto onSubAck =
			[&](Mqtt::MqttConnection &, uint16_t packetId, const String &topic, Mqtt::QOS QoS, int errorCode) {
				if (errorCode)
				{
					fprintf(stderr, "Subscribe failed with error %s\n", aws_error_debug_str(errorCode));
					exit(-1);
				}
				else
				{
					if (!packetId || QoS == AWS_MQTT_QOS_FAILURE)
					{
						fprintf(stderr, "Subscribe rejected by the broker.");
						exit(-1);
					}
					else
					{
						fprintf(stdout, "Subscribe on topic %s on packetId %d Succeeded\n", topic.c_str(), packetId);
					}
				}
				subscribeFinishedPromise.set_value();
			};
#if 0

        if (!subscribeTopic.empty())
        {
            fprintf(stdout, "Subscribing on topic %s ...\n", subscribeTopic.c_str());
            connection->Subscribe(subscribeTopic.c_str(), AWS_MQTT_QOS_AT_LEAST_ONCE, onMessage, onSubAck);
            subscribeFinishedPromise.get_future().wait();
        }
#endif


		while (true)
		{

            sendMessage((char*)"topic_1", (char*)"{\"hello\"}" , 10);

			pthread_cond_wait(&cond_queue, &lock);
			MessageData message = messageQueue.front();
			messageQueue.pop();
			//std::cout << "Got a message : "<< message.c_str()<<std::endl;
			//std::cout << "Got a message : "<< std::endl;

			int length = message.buffer.length();

			char * compressed = new char [length+1];
			char * uncompressed = new char [length+1];

			//Compress Data
			int comp_len = compress(compressType,(char*)message.buffer.c_str(),length, compressed,length+1);
			int decomp_len = decompress(compressType,compressed,comp_len, uncompressed,length+1);


			//Topic Selection
            std::string topicToPublish = "test/topic/publish/";

			topicToPublish.append(thingName.c_str());

			std::cout << "input size "<<length<<std::endl;
			if (compressType != "none")
			{
				std::cout << "compressed size "<<comp_len<<std::endl;
				std::cout << "uncompressed size "<<decomp_len<<std::endl;
			}

			// std::cout << "Compressed Hex : "<<std::endl;
			// for (long int i = 0; i < comp_len; i++)
			// {
			// 	if (i%48 == 0)
			// 		fprintf(stdout, "\n");
			// 	else if (i%16 == 0)
			// 		fprintf(stdout, "    ");
            //     else if (i%8 == 0)
			// 		fprintf(stdout, "  ");
			// 	fprintf(stdout, "%02x",(unsigned char)compressed[i]);
			// }
			// std::cout << std::endl;

			if (compressType != "none")
			{
				fprintf(stdout, "Publishing compressed data(%d) on %s\n", comp_len, topicToPublish.c_str());

				//Publish compressed
				ByteBuf payload = ByteBufNewCopy(DefaultAllocator(), (const uint8_t *)compressed, comp_len);
				ByteBuf *payloadPtr = &payload;

				auto onPublishComplete = [payloadPtr](Mqtt::MqttConnection &, uint16_t packetId, int errorCode) {
					aws_byte_buf_clean_up(payloadPtr);

					if (packetId)
					{
						fprintf(stdout, "Operation on packetId %d Succeeded\n", packetId);
					}
					else
					{
						fprintf(stdout, "Operation failed with error %s\n", aws_error_debug_str(errorCode));
					}
				};
				connection->Publish(topicToPublish.c_str(), AWS_MQTT_QOS_AT_LEAST_ONCE, false, payload, onPublishComplete);

			}
			else
			{
				fprintf(stdout, "Publishing data(%d) on %s\n", decomp_len, topicToPublish.c_str());

				ByteBuf payload = ByteBufNewCopy(DefaultAllocator(), (const uint8_t *)uncompressed, decomp_len);
				ByteBuf *payloadPtr = &payload;

				auto onPublishComplete = [payloadPtr](Mqtt::MqttConnection &, uint16_t packetId, int errorCode) {
					aws_byte_buf_clean_up(payloadPtr);

					if (packetId)
					{
						fprintf(stdout, "Operation on packetId %d Succeeded\n", packetId);
					}
					else
					{
						fprintf(stdout, "Operation failed with error %s\n", aws_error_debug_str(errorCode));
					}
				};
				connection->Publish(topicToPublish.c_str(), AWS_MQTT_QOS_AT_LEAST_ONCE, false, payload, onPublishComplete);
			}

			delete[] compressed;
			delete[] uncompressed;


			time_t currentTime = time(nullptr);
			tm *tm_gmt = gmtime(&currentTime);
			char currentTimeStr[32] ={};
		        sprintf(currentTimeStr,"%02d:%02d:%02d GMT",tm_gmt->tm_hour,tm_gmt->tm_min,tm_gmt->tm_sec);
			fprintf(stdout, "Publishing status on %s\n", publishTopic.c_str());
			char statusmessage[128] = {};
			sprintf(statusmessage,"{\n\t\"message\":\"%s\",\n\t\"timestamp\":%s,\n\t\"epochtime\":%ld,\n\t\"sequence\":%d\n}",clientId.c_str(),currentTimeStr,currentTime,sequenceNumber);


			ByteBuf payload = ByteBufNewCopy(DefaultAllocator(), (const uint8_t *)statusmessage, strlen(statusmessage));
			ByteBuf *payloadPtr = &payload;

			auto onPublishComplete = [payloadPtr](Mqtt::MqttConnection &, uint16_t packetId, int errorCode) {
				aws_byte_buf_clean_up(payloadPtr);

				if (packetId)
				{
					fprintf(stdout, "Operation on packetId %d Succeeded\n", packetId);
				}
				else
				{
					fprintf(stdout, "Operation failed with error %s\n", aws_error_debug_str(errorCode));
				}
			};
			connection->Publish(publishTopic.c_str(), AWS_MQTT_QOS_AT_LEAST_ONCE, false, payload, onPublishComplete);
            fprintf(stdout, "Done \n\n");
            
		}

#if 0
        if (!subscribeTopic.empty())
        {
		/*
		 * Unsubscribe from the topic.
		 */
            std::promise<void> unsubscribeFinishedPromise;
            connection->Unsubscribe(
                    subscribeTopic.c_str(), [&](Mqtt::MqttConnection &, uint16_t, int) { unsubscribeFinishedPromise.set_value(); });
            unsubscribeFinishedPromise.get_future().wait();
        }
#endif
	}
	/* Disconnect */
	if (connection->Disconnect())
	{
		connectionClosedPromise.get_future().wait();
	}

	return 0;
}
