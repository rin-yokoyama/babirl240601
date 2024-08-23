#include <librdkafka/rdkafka.h>

void init_kafka_producer(rd_kafka_t **rk, rd_kafka_topic_t **topic, const char client_id[], const char bootstrap_servers[], const char topic_names[])
{
    char errstr[512];
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    rd_kafka_conf_set(conf, "client.id", client_id, errstr, sizeof(errstr));
    rd_kafka_conf_set(conf, "bootstrap.servers", bootstrap_servers, errstr, sizeof(errstr));
    rd_kafka_topic_conf_t *topic_conf = rd_kafka_topic_conf_new();
    (*rk) = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    (*topic) = rd_kafka_topic_new(*rk, topic_names, topic_conf);
}

void kafka_produce(rd_kafka_topic_t *topic, size_t payload_len, void *payload)
{
    rd_kafka_produce(topic, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY, payload, payload_len, NULL, 0, NULL);
}

void delete_kafka(rd_kafka_t **rk, rd_kafka_topic_t **topic)
{
    rd_kafka_topic_destroy(*topic);
    rd_kafka_destroy(*rk);
    (*topic) = NULL;
    (*rk) = NULL;
}
