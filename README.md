# Telcomanager core dev challenge 


### Requerimentos 
> * versão do Linux kernel 5.8.0-43
> * versão do GNU Make 4.2.1
> * versão do gcc 9.3.0

### Compilando
> make

### Rodando 
> * testado no Ubuntu 20.04.2 LTS

Atualmente é necessário iniciar os programas a partir do path relativo ao root do projeto,
para que as pastas dos clientes sejam criadas corretamente e que não tenha problema de permissões de arquivos.
* Server 
> ./build/dev-core-server
* Client
> ./build/dev-core-client

### Id cadastradas
No projeto existem apenas duas id válidas, 'bob' que já possui uma pasta com arquivos 
e 'test' que tem a sua pasta criada se ela já não existir. 
Qualquer outra id o programa fecha sem dar continuidade.