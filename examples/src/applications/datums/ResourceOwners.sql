create table ResourceOwners (
id char(36) not null primary key,
created timestamp not null,
updated timestamp not null,
href varchar(1024) not null,
created timestamp not null,
type text not null index,
updated timestamp not null
)

