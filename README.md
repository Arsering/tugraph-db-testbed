# 1. 准备工作

## 1.1 拉取并运行TuGraph官方提供的编译环境对应的docker

```bash
# 拉取镜像 (如果镜像已在本地，此步骤可省略)
docker pull tugraph/tugraph-compile-ubuntu18.04
# 新建一个空文件夹 data
# 创建docker
docker run -u uid -dt --oom-kill-disable -v 自定义的文件夹地址(完整地址):在docker中的挂载点（e.g., /data） --name 自定义docker名（建议其中包含你自己的名字，方便辨认） tugraph/tugraph-compile-ubuntu18.04
# 运行docker
docker exec -u uid -it 自定义docker名 bash
```

---

****以下所有的操作皆在上述docker中执行****

---

## 1.2 编译并安装TuGraph的源码 (搬运自TuGraph的Github官网 https://github.com/TuGraph-family/tugraph-db )

### 1.2.1 编译源代码

```bash
git clone --recursive https://github.com/TuGraph-family/tugraph-db.git

cd tugraph-db

deps/build_deps.sh # Using "SKIP_WEB=1 deps/build_deps.sh" to skip building web interface

mkdir build
cd build
cmake .. -DOURSYSTEM=ubuntu # if target OS is centos, run "cmake .. -DOURSYSTEM=centos". If support shell lgraph_cypher, add "-DENABLE_PREDOWNLOAD_DEPENDS_PACKAGE=1"
make -j
make package # or "cpack --config CPackConfig.cmake"
```

### 1.2.2 安装TuGraph

```bash
cd /data/tugraph-db/build
# 使用我们自己编译的可执行文件
# 出现错误：/usr/bin/python: No module named pip 则：curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py && python get-pip.py
dpkg -i tugraph-3.4.0-1.x86_64.deb 
```

## 1.3 配置、编译运行LDBC-SNB所需的环境、源代码（无需重复执行）

### 1.3.1 安装依赖环境

```bash
apt-get install openjdk-8-jre openjdk-8-jdk
python -m pip install requests
```

### 1.3.2 下载需要的源代码

```bash
# 下载ldbc_snb的driver等的源代码
wget https://pub-383410a98aef4cb686f0c7601eddd25f.r2.dev/audits/LDBC_SNB_I_20230128_SF30-100-300_tugraph-attachments.tar.gz
tar -xzf LDBC_SNB_I_20230128_SF30-100-300_tugraph-attachments.tar.gz
cp -r attachments/tugraph_ldbc_snb /data/

# download hadoop
cd /data/tugraph_ldbc_snb/deps
wget http://archive.apache.org/dist/hadoop/core/hadoop-2.9.2/hadoop-2.9.2.tar.gz
tar xf hadoop-2.9.2.tar.gz

# download maven
cd /data/tugraph_ldbc_snb/deps
wget https://repo.maven.apache.org/maven2/org/apache/maven/apache-maven/3.6.0/apache-maven-3.6.0-bin.tar.gz
tar xf apache-maven-3.6.0-bin.tar.gz && export MAVEN_HOME=/data/tugraph_ldbc_snb/deps/apache-maven-3.6.0 && export PATH=${PATH}:${MAVEN_HOME}/bin

# download hopscotch-map & date
cd /data/tugraph_ldbc_snb/deps && git clone https://github.com/Tessil/hopscotch-map
cd /data/tugraph_ldbc_snb/deps && git clone https://github.com/HowardHinnant/date

# downlaod ldbc_snb_datagen_hadoop
# 此处我没有按照官方给的教程调整版本，因为官方使用的版本运行时会存在问题
cd /data/tugraph_ldbc_snb/deps && git clone https://github.com/ldbc/ldbc_snb_datagen_hadoop.git && cd ldbc_snb_datagen_hadoop

# donwload ldbc_snb_interactive_driver
cd /data/tugraph_ldbc_snb/deps && git clone https://github.com/ldbc/ldbc_snb_interactive_driver.git && cd ldbc_snb_interactive_driver && git checkout v1.2.0

# unzip ldbc_snb_interactive_impls
cd /data/tugraph_ldbc_snb/deps
unzip ldbc_snb_interactive_impls.zip

```

### 1.3.3 编译

```bash
cd /data/tugraph_ldbc_snb/deps/ldbc_snb_interactive_driver && mvn install -DskipTests
cd /data/tugraph_ldbc_snb/deps/ldbc_snb_interactive_impls && bash build.sh
```

# 2 运行LDBC-SNB

## 2.1 生成数据

### 2.1.1 设置环境参数

```bash
# Set the following environment variables (you may add these into `~/.profile` for convenience and run `source ~/.profile`)
export CUR_DIR=$(pwd)
export Scale_Factor=sf30
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-amd64
export LD_LIBRARY_PATH=/usr/local/lib64:/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server/
export DB_ROOT_DIR=${CUR_DIR}/lgraph_db/${Scale_Factor}/db
export LC_CTYPE=en_US.UTF-8
# 如果出现：bash: warning: setlocale: LC_ALL: cannot change locale (en_US.UTF-8)，则可使用 https://www.cnblogs.com/cnhack/articles/17418106.html 中的解决方法
# apt-get clean && apt-get -y update && apt-get install -y locales && locale-gen en_US.UTF-8
export LC_ALL=en_US.UTF-8

export HADOOP_HOME="${CUR_DIR}/tugraph_ldbc_snb/deps/hadoop-2.9.2"
export HADOOP_CLIENT_OPTS="-Xms64g -Xmx64g"

export INPUT_OUTPUT_DIR=${CUR_DIR}/experiment-space/LDBC_selfdefined
```

### 2.1.2 生成数据

```bash
cp ${CUR_DIR}/tugraph_ldbc_snb/params-${Scale_Factor}.ini ${CUR_DIR}/tugraph_ldbc_snb/deps/ldbc_snb_datagen_hadoop/params.ini
# 为了能使用多线程提高仿真数据生成的速度，我在原本配置文件的基础上增加了如下的配置
echo "ldbc.snb.datagen.generator.numThreads:128" >> ${CUR_DIR}/tugraph_ldbc_snb/deps/ldbc_snb_datagen_hadoop/params.ini
cd ${CUR_DIR}/tugraph_ldbc_snb/deps/ldbc_snb_datagen_hadoop
# 此命令会生成仿真数据并存储于'/data/tugraph_ldbc_snb/deps/ldbc_snb_datagen_hadoop/social_network'文件夹下
./run.sh

# 将数据转存到其他地方
rm -rf ${CUR_DIR}/lgraph_db/${Scale_Factor}
mkdir ${CUR_DIR}/lgraph_db/${Scale_Factor}
cp -r ${CUR_DIR}/tugraph_ldbc_snb/deps/ldbc_snb_datagen_hadoop/social_network ${CUR_DIR}/lgraph_db/${Scale_Factor}/
mv ${CUR_DIR}/lgraph_db/${Scale_Factor}/social_network ${CUR_DIR}/lgraph_db/${Scale_Factor}/social_network_raw
```

## 2.2 生成原始数据库

### 2.2.1 利用LDBC-SNB生成的数据构建数据库

```bash
# 将上一步生成的仿真数据转换为可存储于TuGraph中的数据格式
# 数据将存于 DB_ROOT_DIR 中
# convert_csvs.sh的第12行改为："python convert.py -i ${DB_ROOT_DIR}/social_network_raw -o import_data"
cd ${CUR_DIR}/tugraph_ldbc_snb
mkdir ${DB_ROOT_DIR}
./convert_csvs.sh

# 第三行修改为：LGRAPH_DB_DIR=${DB_ROOT_DIR}
# 将最后一行修改为：$LGRAPH_IMPORT -c import.conf --dir ${LGRAPH_DB_DIR} --overwrite 1 --online 0
# 每次实验都必须执行一遍
# 执行前清空 ${DB_ROOT_DIR} 文件夹
rm -rf ${DB_ROOT_DIR}
mkdir ${DB_ROOT_DIR}
./import_data.sh # 主要生成构成LMDB的data.mdb和lock.mdb两个文件 
```

### 2.2.2 预处理数据库中的数据

- Generate snb_constants.h

```bash
cd ${CUR_DIR}/tugraph_ldbc_snb/plugins

# 编译 generate_snb_constants.cpp 文件
./compile_embedded.sh generate_snb_constants

# 执行 generate_snb_constants 文件
# generate_snb_constants：主要是生成snb_constants.h文件，该文件中记录了图数据中所有边/顶点的schema信息，比如label name与label id的映射，field id与field name、label id的映射
./generate_snb_constants ${DB_ROOT_DIR} snb_constants.h
```

- Preprocess db

```bash
cd ${CUR_DIR}/tugraph_ldbc_snb/plugins
# 编译 preprocess.cpp 文件
./compile_embedded.sh preprocess
time ./preprocess ${DB_ROOT_DIR}
```

## 2.2.3 安装plugins

- Start lgraph_server

```bash
# 如果lgraph_server已经关闭，则此步骤无意义（忽略报错），可跳过
# 也可以关闭特定的守护进程上的TuGraph，详情见官方文档
lgraph_server -c ${INPUT_OUTPUT_DIR}/configurations/lgraph_standalone.json -d stop --directory ${DB_ROOT_DIR}
# 此命令启动的 TuGraph 服务器进程为守护进程，它将从文件 `lgraph_standalone.json`加载相关配置。服务器启动后，它将开始在日志文件中打印日志，之后可用该日志文件确定服务器的状态
lgraph_server -c ${INPUT_OUTPUT_DIR}/configurations/lgraph_standalone.json -d start --directory ${DB_ROOT_DIR}
```

- Install plugin

```bash
cd ${CUR_DIR}/tugraph_ldbc_snb/plugins
# 编译并将自定义的存储过程加载到TuGraph中
bash install.sh
```

- 创建数据库的副本

```bash
cd ${CUR_DIR}/tugraph_ldbc_snb
# 如果lgraph_server已经关闭，则此步骤无意义（忽略报错），可跳过
# 也可以关闭特定的守护进程上的TuGraph，详情见官方文档
lgraph_server -c ${INPUT_OUTPUT_DIR}/configurations/lgraph_standalone.json -d stop --directory ${DB_ROOT_DIR}

cp -r ${DB_ROOT_DIR} ${DB_ROOT_DIR}_raw
```

# 3 跑Benchmark

## 3.1 配置环境参数

```bash
# Set the following environment variables (you may add these into `~/.profile` for convenience and run `source ~/.profile`)
export CUR_DIR=$(pwd)
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-amd64
export LD_LIBRARY_PATH=/usr/local/lib64:/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server/
export Scale_Factor=sf30
export DB_ROOT_DIR=${CUR_DIR}/lgraph_db/${Scale_Factor}/db
export LC_CTYPE=en_US.UTF-8
# 如果出现：bash: warning: setlocale: LC_ALL: cannot change locale (en_US.UTF-8)，则可使用 https://www.cnblogs.com/cnhack/articles/17418106.html 中的解决方法
export LC_ALL=en_US.UTF-8

export HADOOP_HOME="${CUR_DIR}/tugraph_ldbc_snb/deps/hadoop-2.9.2"
export HADOOP_CLIENT_OPTS="-Xms8g -Xmx8g"
export INPUT_OUTPUT_DIR=${CUR_DIR}/experiment-space/LDBC_selfdefined
```

## 3.2 创建一个干净的db并启动

```bash
rm -rf ${DB_ROOT_DIR}
cp -r ${DB_ROOT_DIR}_raw ${DB_ROOT_DIR}

# 启动db
cd ${CUR_DIR}/tugraph_ldbc_snb
lgraph_server -c ${INPUT_OUTPUT_DIR}/configurations/lgraph_standalone.json -d start --directory ${DB_ROOT_DIR}
```

## 3.3 Run benchmarks

### 3.3.1 validation (sf10)

- Create validation

```bash
# create validation
cd /data/tugraph_ldbc_snb/deps/ldbc_snb_interactive_impls/tugraph && sync
bash run.sh interactive-create-validation-parameters.properties
# stop server
cd /data/tugraph_ldbc_snb
lgraph_server -c ${INPUT_OUTPUT_DIR}/configurations/lgraph_standalone.json -d stop --directory ${DB_ROOT_DIR}
```

- Validate

```bash
# copy (neo4j) validation_params.csv
cd /data/tugraph_ldbc_snb/deps/ldbc_snb_interactive_impls/tugraph && sync
# cp ${VALIDATION_PARAMS_PATH}/validation_params.csv ./

# validate
bash run.sh interactive-validate.properties
# stop server
cd /data/tugraph_ldbc_snb
lgraph_server -c ${INPUT_OUTPUT_DIR}/configurations/lgraph_standalone.json -d stop --directory ${DB_ROOT_DIR}
```

### 3.3.2 Run benchmark (sf30, sf100, sf300)

```bash
# warmup db
# TuGraph 是基于磁盘的数据库，仅当访问数据时，数据才会加载到内存中
lgraph_warmup -d ${DB_ROOT_DIR} -g default
# run benchmark
cd ${CUR_DIR}/tugraph_ldbc_snb/deps/ldbc_snb_interactive_impls/tugraph && sync
export time=$(date "+%Y-%m-%d-%H:%M:%S") && bash run.sh ${INPUT_OUTPUT_DIR}/configurations/interactive-benchmark-test.properties > ${INPUT_OUTPUT_DIR}/logs/${time}.log

# stop server
cd ${CUR_DIR}/tugraph_ldbc_snb
lgraph_server -c ${INPUT_OUTPUT_DIR}/configurations/lgraph_standalone.json -d stop --directory ${DB_ROOT_DIR}
```

### 3.3.3 Check consistency

```bash
cd /data/tugraph_ldbc_snb/plugins
./compile_embedded.sh check_consistency
./check_consistency ${DB_ROOT_DIR}
```
