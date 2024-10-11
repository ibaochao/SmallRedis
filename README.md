# SmallRedis

## 简介

技术支持：C++、 CMake、 C/S 模式、 Linux 环境、 RPC 框架、 跳表等。

功能： 

- 自定义字符串、列表、哈希表等多种值类型； 

- 跳表实现数据库底层数据结构； 

- 包含近 30 种操作命令如 select、set、 get、 mset、 incr、 rpush、 hset 等； 

- 可自由切换不同序号数据库； 

- 数据持久化至文件以及从文件中解析数据。  

## 编译启动

编译，依次执行以下Linux命令

```txt
mkdir build
cd build
cmake ..
make
```

项目目录结构，编译后

![image-20241011131858326](./SmallRedis.assets/image-20241011131858326.png)

bin目录

![image-20241011132235935](./SmallRedis.assets/image-20241011132235935.png)

build目录

![image-20241011132305754](./SmallRedis.assets/image-20241011132305754.png)

data_files目录（测试后补）

![image-20241011132120665](./SmallRedis.assets/image-20241011132120665.png)

src目录

![image-20241011132424494](./SmallRedis.assets/image-20241011132424494.png)

启动，依次执行以下Linux命令

```txt
// 跳转至项目根目录
cd ..
// 启动server服务端
./bin/server
// 启动client服务端
./bin/client
```

server服务端启动

![image-20241011121827774](./SmallRedis.assets/image-20241011121827774.png)

client客户端启动

![image-20241011121847347](./SmallRedis.assets/image-20241011121847347.png)

## 测试命令

### 常规命令

#### select

切换数据库， 0-15

```c++
set k0 v0
select 10
set k10 v10
select 0
get k0
```

![image-20241011121942980](./SmallRedis.assets/image-20241011121942980.png)

#### keys

获取全部key

```c++
set k00 v00
keys *
```

![image-20241011122024880](./SmallRedis.assets/image-20241011122024880.png)

#### dbsize

```c++
dbsize
```

![image-20241011122045348](./SmallRedis.assets/image-20241011122045348.png)

#### exists

```c++
exists k0
exists k000
```

![image-20241011122115963](./SmallRedis.assets/image-20241011122115963.png)

#### del

```c++
del k0 k00
```

![image-20241011122149563](./SmallRedis.assets/image-20241011122149563.png)

#### rename

```c++
set k0 v0
get k0
rename k0 key0
get k0
get key0
```

![image-20241011122337342](./SmallRedis.assets/image-20241011122337342.png)

#### exit

```txt
exit  // 退出客户端
```

![image-20241011131950341](./SmallRedis.assets/image-20241011131950341.png)

### 字符串命令

#### set

```txt
select 2
keys *
set k1 v1  // 默认，不存在则添加，存在则覆盖
set k1 v1 NX  // 存在则报错
set k2 v2 NX  // 不存在则添加
set k3 v3 XX  // 不存在则报错
set k1 v11 XX  // 存在则覆盖
```

![image-20241011124502445](./SmallRedis.assets/image-20241011124502445.png)

#### setnx

```txt
setnx k6 v6
setnx k6 v6
```

![image-20241011124603819](./SmallRedis.assets/image-20241011124603819.png)

#### setex

```c++
setex k7 v7
setex k6 v66
get k6
```

![image-20241011124639318](./SmallRedis.assets/image-20241011124639318.png)

#### get

```c++
get k6
```

![image-20241011124713912](./SmallRedis.assets/image-20241011124713912.png)

#### incr

```c++
set num 5
incr num
```

![image-20241011124746832](./SmallRedis.assets/image-20241011124746832.png)

#### incrby

```c++
get num
incrby num 3
```

![image-20241011124847681](./SmallRedis.assets/image-20241011124847681.png)

#### incrbyfloat

```c++
set fnum 3.14
get fnum
incrbyfloat fnum 0.6
```

![image-20241011124928412](./SmallRedis.assets/image-20241011124928412.png)

#### decr

```c++
get num
decr num
```

![image-20241011124950602](./SmallRedis.assets/image-20241011124950602.png)

#### decrby

```c++
get num
decrby num 4
```

![image-20241011125019832](./SmallRedis.assets/image-20241011125019832.png)

#### mset

```txt
select 3
keys *
mset k1 v1 k2 v2 k3 v3
get k1
get k2
get k3
```

![image-20241011125129168](./SmallRedis.assets/image-20241011125129168.png)

#### mget

```c++
mget k1 k2 k3
```

![image-20241011125146695](./SmallRedis.assets/image-20241011125146695.png)

#### strlen

```c++
set str Hello
strlen str
```

![image-20241011125657604](./SmallRedis.assets/image-20241011125657604.png)

不太对，前后字符也算上了

#### append

```c++
append str China
get str
```

![image-20241011125846034](./SmallRedis.assets/image-20241011125846034.png)

### 列表命令

#### lpush

```txt
select 5
keys *
lpush l1 e1
lpush l1 e2
get l1
```

![image-20241011130345579](./SmallRedis.assets/image-20241011130345579.png)

#### rpush

```txt
get l1
rpush l1 e3
rpush l1 e4
get l1
```

![image-20241011130421913](./SmallRedis.assets/image-20241011130421913.png)

#### lpop

```txt
get l1
lpop l1
get l1
```

![image-20241011130504508](./SmallRedis.assets/image-20241011130504508.png)

#### rpop

```txt
get l1
rpop l1
get l1
```

![image-20241011130529777](./SmallRedis.assets/image-20241011130529777.png)

#### lrange

```txt
rpush l2 e1
rpush l2 e2
rpush l2 e3
rpush l2 e4
rpush l2 e5
rpush l2 e6
lrange l2 1 4
```

![image-20241011130648331](./SmallRedis.assets/image-20241011130648331.png)

### 哈希表命令

#### hset

```txt
select 8
keys *
hset h1 name zhangsan
get h1
```

![image-20241011131316836](./SmallRedis.assets/image-20241011131316836.png)

#### hget

```c++
hget h1 name
```

![image-20241011131335606](./SmallRedis.assets/image-20241011131335606.png)

#### hdel

```txt
get h1
hset h1 age 22
hset h1 sex male
get h1
hdel h1 age
get h1
```

![image-20241011131545646](./SmallRedis.assets/image-20241011131545646.png)

#### hkeys

```c++
get h1
hkeys h1
```

![image-20241011131605059](./SmallRedis.assets/image-20241011131605059.png)

#### hvals

```c++
get h1
hvals h1
```

![image-20241011131630288](./SmallRedis.assets/image-20241011131630288.png)

## 参考

```c++
跳表: https://leetcode.cn/problems/design-skiplist/description/
TinyRedis: https://github.com/zk1556/TinyRedis_RPC
```

# End







