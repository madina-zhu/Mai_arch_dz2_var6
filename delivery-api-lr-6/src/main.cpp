#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/postgres/component.hpp>
// #include <userver/storages/redis/component.hpp>  // <-- ОТКЛЮЧЕН
// #include <userver/storages/secdist/component.hpp>  // <-- ОТКЛЮЧЕН
// #include <userver/storages/secdist/provider_component.hpp>  // <-- ОТКЛЮЧЕН
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
// #include <userver/urabbitmq/component.hpp>  // <-- ОТКЛЮЧЕН

#include "components/auth_component.hpp"
// #include "components/delivery_event_consumer.hpp"  // <-- ОТКЛЮЧЕН
// #include "components/delivery_event_producer.hpp"  // <-- ОТКЛЮЧЕН
#include "components/mongo_storage_component.hpp"
#include "components/postgres_storage_component.hpp"
#include "components/rate_limiter_component.hpp"
#include "components/read_database_component.hpp"
// #include "components/redis_cache_component.hpp"  // <-- ОТКЛЮЧЕН
#include "handlers/create_delivery_handler.hpp"
#include "handlers/create_parcel_handler.hpp"
#include "handlers/create_user_handler.hpp"
#include "handlers/get_deliveries_handler.hpp"
#include "handlers/get_parcels_handler.hpp"
#include "handlers/get_users_handler.hpp"
#include "handlers/login_handler.hpp"
#include "handlers/openapi_handler.hpp"
#include "handlers/swagger_ui_handler.hpp"
#include "jwt_auth/jwt_auth_checker.hpp"
#include "jwt_auth/jwt_auth_factory.hpp"

int main(int argc, char *argv[])
{
    userver::server::handlers::auth::RegisterAuthCheckerFactory<
        auth::jwt::JwtAuthCheckerFactory>();

    auto component_list = userver::components::MinimalServerComponentList()
                              .Append<userver::server::handlers::Ping>()
                              .Append<userver::components::TestsuiteSupport>()
                              .Append<userver::clients::dns::Component>()
                              .Append<userver::components::Postgres>("db-postgresql")
                              .Append<userver::components::Mongo>("mongo")
                              // .Append<userver::components::Redis>("redis")  // <-- ОТКЛЮЧЕН
                              // .Append<userver::components::RabbitMQ>("rabbit")  // <-- ОТКЛЮЧЕН
                              // .Append<userver::components::DefaultSecdistProvider>()  // <-- ОТКЛЮЧЕН
                              // .Append<userver::components::Secdist>()  // <-- ОТКЛЮЧЕН
                              .Append<components::RateLimiterComponent>("rate-limit-component")
                              .Append<components::PostgresStorageComponent>("postgres-storage-component")
                              .Append<components::MongoStorageComponent>("mongo-storage-component")
                              .Append<components::ReadDatabaseComponent>("read-database-component")
                              // .Append<components::RedisCacheComponent>("redis-cache-component")  // <-- ОТКЛЮЧЕН
                              // .Append<components::DeliveryEventProducer>("delivery-event-producer")  // <-- ОТКЛЮЧЕН
                              // .Append<components::DeliveryEventConsumer>("delivery-event-consumer")  // <-- ОТКЛЮЧЕН
                              .Append<components::AuthComponent>("auth-component")
                              .Append<auth::jwt::JwtAuthComponent>()
                              .Append<handlers::CreateUserHandler>()
                              .Append<handlers::GetUsersHandler>()
                              .Append<handlers::LoginHandler>()
                              .Append<handlers::CreateParcelHandler>()
                              .Append<handlers::GetParcelsHandler>()
                              .Append<handlers::CreateDeliveryHandler>()
                              .Append<handlers::GetDeliveriesHandler>()
                              .Append<handlers::OpenApiHandler>()
                              .Append<handlers::SwaggerUiHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}