pipeline {
    agent any
    stages {
        stage('Build Binaries') {
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
                    sh 'docker run --name ubuntu_dev_CI -dit -p 6200:6200 -p 27017:27017 ubuntu-development:1.0 /bin/bash'
                    sh  label:
                    'Update Repo and start Mongod',
                    script:'''
                    echo "Update Repo and start Mongod"
                    set -x
                    docker exec -w /Development/ ubuntu_dev_CI pwd
                    docker exec -w /Development/ ubuntu_dev_CI ls -l
                    docker exec -w /Development/ ubuntu_dev_CI git pull
                    docker exec -w /Development/Milestone3/ ubuntu_dev_CI sudo mongod --port 27017 --dbpath /srv/mongodb/db0 --replSet rs0 --bind_ip localhost --fork --logpath /var/log/mongod.log
                    docker exec -w /Development/Milestone3/ ubuntu_dev_CI ps -ef
                    '''
                }
                script {
                    echo 'Build Binaries'
                    sh label:
                    'Build Binaries',
                    script:'''
                    set -x
                    docker exec -w /Development/Milestone3/ ubuntu_dev_CI sh CreateDailyBuild.sh
                    docker exec -w /Development/Milestone3/Binary ubuntu_dev_CI sh -c "ls -l"
                    docker exec -w /Development/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseGateway  > database.log &"
                    sleep 1
                    docker exec -w /Development/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./RestApiPortal > portal.log &"
                    sleep 1
                    docker exec -w /Development/Milestone3/ ubuntu_dev_CI ps -ef
                    # docker stop ubuntu_dev_CI
                    # docker rm ubuntu_dev_CI
                    # docker kill $(docker ps -q)
                    # docker rm $(docker ps -a -q)
                    '''
                }
                script {
                    try {
                        echo 'Load Database'
                        sh 'docker exec -w /Development/Milestone3/Binary ubuntu_dev_CI sh -c "ls -l"'
                        sh 'docker exec -w /Development/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200"'
                    }catch (exception) {
                        echo getStackTrace(exception)
                        echo 'Error detected, retrying...'
                        sh '''
                            docker exec -w /Development/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200 -d"
                            docker exec -w /Development/Milestone3/Binary ubuntu_dev_CI sh -c "sudo ./DatabaseTools --PortalIp=127.0.0.1 --Port=6200"
                            ''' 
                    }
                }
                echo 'Backend Portal Server is Deployed and Ready to use'
                echo 'Build Successful'
                echo 'Teardown'
                script {
                    echo 'Teardown'
                    sh label:
                    'Teardown',
                    script:'''
                    set -x
                    docker stop ubuntu_dev_CI
                    docker rm ubuntu_dev_CI
                    '''
                }
            }
        }
    }
}
