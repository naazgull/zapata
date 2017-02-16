create table Applications (
id char(36) not null primary key,
created timestamp not null,
updated timestamp not null,
href varchar(1024) not null,
app_secret varchar(512) not null index,
cloud_secret varchar(512) not null index,
device_secret varchar(512) not null index,
icon varchar(512) ,
image varchar(512) ,
name text not null,
namespace text not null index
)

