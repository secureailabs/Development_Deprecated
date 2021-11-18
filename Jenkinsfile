pipeline {
    agent any
    options {
    buildDiscarder(logRotator(numToKeepStr: '30', artifactNumToKeepStr: '30'))
    parallelsAlwaysFailFast()
    timestamps()
    }
    stages {
        stage('Git') {
            steps {
                pwd(tmp: true)
                sh '''
                pwd
                ls -l
                '''
                echo 'Starting to build docker image: Backend Api Portal Server'
                script {
                    docker.build('ubuntu-development:1.0', '--build-arg git_personal_token=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 -f Dockerfile.development .')
                    sh 'pwd'
                    sh 'docker run --name ubuntu_dev_CI -dit -p 6200:6200 -p 27017:27017 -v ${PWD}:/Workspace -w="/Workspace" ubuntu-development:1.0 /bin/bash'
                    sh  label:
                    'Update Repo and start Mongod',
                    script:'''
                    echo "Update Repo and start Mongod"
                    set -x
                    docker exec -w /Development/ ubuntu_dev_CI pwd
                    docker exec -w /Development/ ubuntu_dev_CI ls -l
                    docker exec -w /Development/ ubuntu_dev_CI git pull
                    docker exec -w /Workspace/Milestone3/ ubuntu_dev_CI rm -rf Binary/
                    docker exec -w /Workspace/Milestone5/ ubuntu_dev_CI rm -rf Binary/
                    docker exec -w /Development/Milestone3/ ubuntu_dev_CI sudo mongod --port 27017 --dbpath /srv/mongodb/db0 --replSet rs0 --bind_ip localhost --fork --logpath /var/log/mongod.log
                    docker exec -w /Development/Milestone3/ ubuntu_dev_CI ps -ef
                    '''
                }
            }
            post {
                failure {
                    echo "Failed during Git stage"
                }
            }
        }
        stage('Build Backend3') {
            steps {
                script {
                    echo 'Build Binaries'
                    sh label:
                    'Build Binaries',
                    script:'''
                    set -x
                    docker exec -w /Workspace/Milestone3/ ubuntu_dev_CI ./CreateDailyBuild.sh
                    docker exec -w /Workspace/Milestone3/Binary ubuntu_dev_CI sh -c "ls -l"
                    '''
                }
            }
            post {
                failure {
                    echo "Failed during Build Backend stage"
                }
            }
        }
        stage ('Deploy Backend3') {
            steps {
                script {
                    echo 'Deploy DatabaseGateway and RestApiPortal'
                    sh '''
                    docker exec -w /Workspace/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseGateway  > database.log &"
                    sleep 1
                    docker exec -w /Workspace/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./RestApiPortal > portal.log &"
                    sleep 1
                    docker exec -w /Workspace/Milestone3/ ubuntu_dev_CI ps -ef
                    '''
                }
                script {
                    try {
                        echo 'Load Database'
                        sh 'docker exec -w /Workspace/Milestone3/Binary ubuntu_dev_CI sh -c "ls -l"'
                        sh 'docker exec -w /Workspace/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200"'
                    }catch (exception) {
                        echo getStackTrace(exception)
                        echo 'Error detected, retrying...'
                        sh '''
                        docker exec -w /Workspace/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200 -d"
                        docker exec -w /Workspace/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200"
                        ''' 
                    }
                }
                echo 'Backend Portal Server is Deployed and Ready to use'
                echo 'Build Successful'
                sh '''
                ps -ef | egrep "DatabaseGateway|RestApiPortal"
                killall -9 DatabaseGateway &&  killall -9 RestApiPortal
                '''
            }
            post {
                failure {
                    echo "Failed during Deploy Backend stage"
                }
            }
        }
        stage('Build Backend5') {
            steps {
                script {
                    echo 'Build Binaries'
                    sh label:
                    'Build Binaries',
                    script:'''
                    set -x
                    docker exec -w /Workspace/Milestone5/ ubuntu_dev_CI ./CreateDailyBuild.sh
                    docker exec -w /Workspace/Milestone5/Binary ubuntu_dev_CI sh -c "ls -l"
                    '''
                }
            }
            post {
                failure {
                    echo "Failed during Build Backend stage"
                }
            }
        }
        stage ('Deploy Backend5') {
            steps {
                script {
                    echo 'Deploy DatabaseGateway and RestApiPortal'
                    sh '''
                    docker exec -w /Workspace/Milestone5/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseGateway  > database.log &"
                    sleep 1
                    docker exec -w /Workspace/Milestone5/Binary ubuntu_dev_CI sh -c "sudo ./RestApiPortal > portal.log &"
                    sleep 1
                    docker exec -w /Workspace/Milestone5/ ubuntu_dev_CI ps -ef
                    '''
                }
                script {
                    try {
                        echo 'Load Database'
                        sh 'docker exec -w /Workspace/Milestone5/Binary ubuntu_dev_CI sh -c "ls -l"'
                        sh 'docker exec -w /Workspace/Milestone5/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200"'
                    }catch (exception) {
                        echo getStackTrace(exception)
                        echo 'Error detected, retrying...'
                        sh '''
                        docker exec -w /Workspace/Milestone5/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200 -d"
                        docker exec -w /Workspace/Milestone5/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200"
                        ''' 
                    }
                }
                echo 'Backend Portal Server is Deployed and Ready to use'
                echo 'Build Successful'
                sh '''
                ps -ef | egrep "DatabaseGateway|RestApiPortal"
                killall -9  DatabaseGateway &&  killall -9 RestApiPortal
                '''
            }
            post {
                failure {
                    echo "Failed during Deploy Backend stage"
                }
            }
        }
    }
    post {
        always {
            echo 'Teardown'
            sh label:
            'Teardown',
            script:'''
            set -x
            docker kill $(docker ps -q)
            docker rm $(docker ps -a -q)
            '''
        }
    }
}
