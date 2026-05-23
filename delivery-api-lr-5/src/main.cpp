#include "./components/auth_component.hpp"
#include "./components/mongo_storage_component.hpp"
#include "./components/postgres_storage_component.hpp"
#include "./components/rate_limiter_component.hpp"
#include "./components/redis_cache_component.hpp"
#include "./jwt_auth/jwt_auth_checker.hpp"
#include "./jwt_auth/jwt_auth_factory.hpp"
#include "handlers/create_delivery_handler.hpp"
#include "handlers/create_parcel_handler.hpp"
#include "handlers/create_user_handler.hpp"
#include "handlers/get_deliveries_handler.hpp"
#include "handlers/get_parcels_handler.hpp"
#include "handlers/get_users_handler.hpp"
#include "handlers/login_handler.hpp"
#include "handlers/openapi_handler.hpp"
#include "handlers/swagger_ui_handler.hpp"

#include <userver/clients/dns/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/components/run.hpp>
#include <userver/server/handlers/auth/auth_checker_factory.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/redis/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

int main(int argc, char *argv[])
{
    userver::server::handlers::auth::RegisterAuthCheckerFactory<
        auth::jwt::JwtAuthCheckerFactory>();

    auto component_list = userver::components::MinimalServerComponentList()
                              .Append<userver::server::handlers::Ping>()
                              .Append<userver::clients::dns::Component>()
                              .Append<userver::components::TestsuiteSupport>()
                              .Append<userver::components::Postgres>("db-postgresql")
                              .Append<userver::components::Mongo>("mongo")
                              .Append<userver::components::Redis>("redis")
                              .Append<userver::components::DefaultSecdistProvider>()
                              .Append<userver::components::Secdist>()
                              .Append<components::RateLimiterComponent>("rate-limit-component")
                              .Append<components::PostgresStorageComponent>("postgres-storage-component")
                              .Append<components::MongoStorageComponent>("mongo-storage-component")
                              .Append<components::RedisCacheComponent>("redis-cache-component")
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