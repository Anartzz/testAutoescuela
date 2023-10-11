drop table colecciones;
drop table tests;
drop table preguntas;
drop table respuestas;

create table colecciones (
    id int(1) primary key, 
    nombre varchar(20)
);

create table tests (
    id int(2) primary key, 
    idColecciones int(1) references colecciones(id)
);

create table preguntas (
    id int(2) primary key, 
    idTests int(1) references tests(id),
    pregunta varchar(100),
    ayuda varchar(1000)
);

create table respuestas (
    id int(1) auto_increment primary key,
    idPreuntas int(1) references preguntas(id),
    respuesta varchar(100),
    correcta int(1)
);



