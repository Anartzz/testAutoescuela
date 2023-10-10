drop table colecciones;
drop table tests;
drop table preguntas;
drop table respuestas;

create table colecciones (
    id int(1), 
    nombre varchar(20)
);

create table tests (
    id int(2), 
    idColeciones int(1) references colecciones(id)
);

create table preguntas (
    id int(2), 
    idTests int(1) references tests(id),
    pregunta varchar(100)
);

create table respuestas (
    id int(1),
    idPreuntas int(1) references preguntas(id),
    respuesta varchar(100)
);



