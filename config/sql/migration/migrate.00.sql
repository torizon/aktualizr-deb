-- Don't modify this! Create a new migration instead--see docs/ota-client-guide/modules/ROOT/pages/schema-migrations.adoc
SAVEPOINT MIGRATION;

CREATE TABLE version(version INTEGER);

CREATE TABLE device_info(device_id TEXT NOT NULL, is_registered INTEGER NOT NULL CHECK (is_registered IN (0,1)));
CREATE TABLE ecu_serials(serial TEXT UNIQUE, hardware_id TEXT NOT NULL, is_primary INTEGER NOT NULL CHECK (is_primary IN (0,1)));
CREATE TABLE misconfigured_ecus(serial TEXT UNIQUE, hardware_id TEXT NOT NULL, state INTEGER NOT NULL CHECK (state IN (0,1)));
CREATE TABLE primary_keys(private TEXT, public TEXT);
CREATE TABLE installed_versions(hash TEXT UNIQUE, name TEXT NOT NULL);

CREATE TABLE tls_creds(ca_cert BLOB, ca_cert_format TEXT,
                       client_cert BLOB, client_cert_format TEXT,
                       client_pkey BLOB, client_pkey_format TEXT);
CREATE TABLE root_meta(root BLOB NOT NULL, root_format TEXT NOT NULL, director INTEGER NOT NULL CHECK (director IN (0,1)), version INTEGER NOT NULL);
CREATE TABLE meta(director_root BLOB NOT NULL,
                  director_targets BLOB NOT NULL,
                  image_root BLOB NOT NULL,
                  image_targets BLOB NOT NULL,
                  image_timestamp BLOB NOT NULL,
                  image_snapshot BLOB NOT NULL);
CREATE TABLE primary_image(filepath TEXT NOT NULL);

INSERT INTO version VALUES(0);
RELEASE MIGRATION;
