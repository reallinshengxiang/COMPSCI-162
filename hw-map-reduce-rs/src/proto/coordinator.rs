#[derive(Clone, PartialEq, ::prost::Message)]
pub struct ExampleRequest {
    #[prost(string, tag="1")]
    pub name: ::prost::alloc::string::String,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct ExampleReply {
    #[prost(string, tag="1")]
    pub message: ::prost::alloc::string::String,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct SubmitJobRequest {
    #[prost(string, repeated, tag="1")]
    pub files: ::prost::alloc::vec::Vec<::prost::alloc::string::String>,
    #[prost(string, tag="2")]
    pub output_dir: ::prost::alloc::string::String,
    #[prost(string, tag="3")]
    pub app: ::prost::alloc::string::String,
    #[prost(uint32, tag="4")]
    pub n_reduce: u32,
    #[prost(bytes="vec", tag="5")]
    pub args: ::prost::alloc::vec::Vec<u8>,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct SubmitJobReply {
    #[prost(uint32, tag="1")]
    pub job_id: u32,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct PollJobRequest {
    #[prost(uint32, tag="1")]
    pub job_id: u32,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct PollJobReply {
    #[prost(bool, tag="1")]
    pub done: bool,
    #[prost(bool, tag="2")]
    pub failed: bool,
    #[prost(string, repeated, tag="3")]
    pub errors: ::prost::alloc::vec::Vec<::prost::alloc::string::String>,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct HeartbeatRequest {
    #[prost(uint32, tag="1")]
    pub worker_id: u32,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct HeartbeatReply {
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct RegisterRequest {
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct RegisterReply {
    #[prost(uint32, tag="1")]
    pub worker_id: u32,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct GetTaskRequest {
    #[prost(uint32, tag="1")]
    pub worker_id: u32,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct MapTaskAssignment {
    #[prost(uint32, tag="1")]
    pub task: u32,
    #[prost(uint32, tag="2")]
    pub worker_id: u32,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct GetTaskReply {
    #[prost(uint32, tag="1")]
    pub job_id: u32,
    #[prost(string, tag="2")]
    pub output_dir: ::prost::alloc::string::String,
    #[prost(string, tag="3")]
    pub app: ::prost::alloc::string::String,
    #[prost(uint32, tag="4")]
    pub task: u32,
    #[prost(string, tag="5")]
    pub file: ::prost::alloc::string::String,
    #[prost(uint32, tag="6")]
    pub n_reduce: u32,
    #[prost(uint32, tag="7")]
    pub n_map: u32,
    #[prost(bool, tag="8")]
    pub reduce: bool,
    #[prost(bool, tag="9")]
    pub wait: bool,
    #[prost(message, repeated, tag="10")]
    pub map_task_assignments: ::prost::alloc::vec::Vec<MapTaskAssignment>,
    #[prost(bytes="vec", tag="11")]
    pub args: ::prost::alloc::vec::Vec<u8>,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct FinishTaskRequest {
    #[prost(uint32, tag="1")]
    pub worker_id: u32,
    #[prost(uint32, tag="2")]
    pub job_id: u32,
    #[prost(uint32, tag="3")]
    pub task: u32,
    #[prost(bool, tag="4")]
    pub reduce: bool,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct FinishTaskReply {
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct FailTaskRequest {
    #[prost(uint32, tag="1")]
    pub worker_id: u32,
    #[prost(uint32, tag="2")]
    pub job_id: u32,
    #[prost(uint32, tag="3")]
    pub task: u32,
    #[prost(bool, tag="4")]
    pub reduce: bool,
    #[prost(bool, tag="5")]
    pub retry: bool,
    #[prost(string, tag="6")]
    pub error: ::prost::alloc::string::String,
}
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct FailTaskReply {
}
/// Generated client implementations.
pub mod coordinator_client {
    #![allow(unused_variables, dead_code, missing_docs, clippy::let_unit_value)]
    use tonic::codegen::*;
    #[derive(Debug, Clone)]
    pub struct CoordinatorClient<T> {
        inner: tonic::client::Grpc<T>,
    }
    impl CoordinatorClient<tonic::transport::Channel> {
        /// Attempt to create a new client by connecting to a given endpoint.
        pub async fn connect<D>(dst: D) -> Result<Self, tonic::transport::Error>
        where
            D: std::convert::TryInto<tonic::transport::Endpoint>,
            D::Error: Into<StdError>,
        {
            let conn = tonic::transport::Endpoint::new(dst)?.connect().await?;
            Ok(Self::new(conn))
        }
    }
    impl<T> CoordinatorClient<T>
    where
        T: tonic::client::GrpcService<tonic::body::BoxBody>,
        T::Error: Into<StdError>,
        T::ResponseBody: Body<Data = Bytes> + Send + 'static,
        <T::ResponseBody as Body>::Error: Into<StdError> + Send,
    {
        pub fn new(inner: T) -> Self {
            let inner = tonic::client::Grpc::new(inner);
            Self { inner }
        }
        pub fn with_interceptor<F>(
            inner: T,
            interceptor: F,
        ) -> CoordinatorClient<InterceptedService<T, F>>
        where
            F: tonic::service::Interceptor,
            T::ResponseBody: Default,
            T: tonic::codegen::Service<
                http::Request<tonic::body::BoxBody>,
                Response = http::Response<
                    <T as tonic::client::GrpcService<tonic::body::BoxBody>>::ResponseBody,
                >,
            >,
            <T as tonic::codegen::Service<
                http::Request<tonic::body::BoxBody>,
            >>::Error: Into<StdError> + Send + Sync,
        {
            CoordinatorClient::new(InterceptedService::new(inner, interceptor))
        }
        /// Compress requests with `gzip`.
        ///
        /// This requires the server to support it otherwise it might respond with an
        /// error.
        #[must_use]
        pub fn send_gzip(mut self) -> Self {
            self.inner = self.inner.send_gzip();
            self
        }
        /// Enable decompressing responses with `gzip`.
        #[must_use]
        pub fn accept_gzip(mut self) -> Self {
            self.inner = self.inner.accept_gzip();
            self
        }
        /// An example RPC, to show you how to use the RPC framework.
        /// You should delete this RPC and add your own.
        pub async fn example(
            &mut self,
            request: impl tonic::IntoRequest<super::ExampleRequest>,
        ) -> Result<tonic::Response<super::ExampleReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/Example",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
        pub async fn submit_job(
            &mut self,
            request: impl tonic::IntoRequest<super::SubmitJobRequest>,
        ) -> Result<tonic::Response<super::SubmitJobReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/SubmitJob",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
        pub async fn poll_job(
            &mut self,
            request: impl tonic::IntoRequest<super::PollJobRequest>,
        ) -> Result<tonic::Response<super::PollJobReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/PollJob",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
        pub async fn heartbeat(
            &mut self,
            request: impl tonic::IntoRequest<super::HeartbeatRequest>,
        ) -> Result<tonic::Response<super::HeartbeatReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/Heartbeat",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
        pub async fn register(
            &mut self,
            request: impl tonic::IntoRequest<super::RegisterRequest>,
        ) -> Result<tonic::Response<super::RegisterReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/Register",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
        pub async fn get_task(
            &mut self,
            request: impl tonic::IntoRequest<super::GetTaskRequest>,
        ) -> Result<tonic::Response<super::GetTaskReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/GetTask",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
        pub async fn finish_task(
            &mut self,
            request: impl tonic::IntoRequest<super::FinishTaskRequest>,
        ) -> Result<tonic::Response<super::FinishTaskReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/FinishTask",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
        pub async fn fail_task(
            &mut self,
            request: impl tonic::IntoRequest<super::FailTaskRequest>,
        ) -> Result<tonic::Response<super::FailTaskReply>, tonic::Status> {
            self.inner
                .ready()
                .await
                .map_err(|e| {
                    tonic::Status::new(
                        tonic::Code::Unknown,
                        format!("Service was not ready: {}", e.into()),
                    )
                })?;
            let codec = tonic::codec::ProstCodec::default();
            let path = http::uri::PathAndQuery::from_static(
                "/coordinator.Coordinator/FailTask",
            );
            self.inner.unary(request.into_request(), path, codec).await
        }
    }
}
/// Generated server implementations.
pub mod coordinator_server {
    #![allow(unused_variables, dead_code, missing_docs, clippy::let_unit_value)]
    use tonic::codegen::*;
    ///Generated trait containing gRPC methods that should be implemented for use with CoordinatorServer.
    #[async_trait]
    pub trait Coordinator: Send + Sync + 'static {
        /// An example RPC, to show you how to use the RPC framework.
        /// You should delete this RPC and add your own.
        async fn example(
            &self,
            request: tonic::Request<super::ExampleRequest>,
        ) -> Result<tonic::Response<super::ExampleReply>, tonic::Status>;
        async fn submit_job(
            &self,
            request: tonic::Request<super::SubmitJobRequest>,
        ) -> Result<tonic::Response<super::SubmitJobReply>, tonic::Status>;
        async fn poll_job(
            &self,
            request: tonic::Request<super::PollJobRequest>,
        ) -> Result<tonic::Response<super::PollJobReply>, tonic::Status>;
        async fn heartbeat(
            &self,
            request: tonic::Request<super::HeartbeatRequest>,
        ) -> Result<tonic::Response<super::HeartbeatReply>, tonic::Status>;
        async fn register(
            &self,
            request: tonic::Request<super::RegisterRequest>,
        ) -> Result<tonic::Response<super::RegisterReply>, tonic::Status>;
        async fn get_task(
            &self,
            request: tonic::Request<super::GetTaskRequest>,
        ) -> Result<tonic::Response<super::GetTaskReply>, tonic::Status>;
        async fn finish_task(
            &self,
            request: tonic::Request<super::FinishTaskRequest>,
        ) -> Result<tonic::Response<super::FinishTaskReply>, tonic::Status>;
        async fn fail_task(
            &self,
            request: tonic::Request<super::FailTaskRequest>,
        ) -> Result<tonic::Response<super::FailTaskReply>, tonic::Status>;
    }
    #[derive(Debug)]
    pub struct CoordinatorServer<T: Coordinator> {
        inner: _Inner<T>,
        accept_compression_encodings: (),
        send_compression_encodings: (),
    }
    struct _Inner<T>(Arc<T>);
    impl<T: Coordinator> CoordinatorServer<T> {
        pub fn new(inner: T) -> Self {
            Self::from_arc(Arc::new(inner))
        }
        pub fn from_arc(inner: Arc<T>) -> Self {
            let inner = _Inner(inner);
            Self {
                inner,
                accept_compression_encodings: Default::default(),
                send_compression_encodings: Default::default(),
            }
        }
        pub fn with_interceptor<F>(
            inner: T,
            interceptor: F,
        ) -> InterceptedService<Self, F>
        where
            F: tonic::service::Interceptor,
        {
            InterceptedService::new(Self::new(inner), interceptor)
        }
    }
    impl<T, B> tonic::codegen::Service<http::Request<B>> for CoordinatorServer<T>
    where
        T: Coordinator,
        B: Body + Send + 'static,
        B::Error: Into<StdError> + Send + 'static,
    {
        type Response = http::Response<tonic::body::BoxBody>;
        type Error = std::convert::Infallible;
        type Future = BoxFuture<Self::Response, Self::Error>;
        fn poll_ready(
            &mut self,
            _cx: &mut Context<'_>,
        ) -> Poll<Result<(), Self::Error>> {
            Poll::Ready(Ok(()))
        }
        fn call(&mut self, req: http::Request<B>) -> Self::Future {
            let inner = self.inner.clone();
            match req.uri().path() {
                "/coordinator.Coordinator/Example" => {
                    #[allow(non_camel_case_types)]
                    struct ExampleSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::ExampleRequest>
                    for ExampleSvc<T> {
                        type Response = super::ExampleReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::ExampleRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).example(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = ExampleSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                "/coordinator.Coordinator/SubmitJob" => {
                    #[allow(non_camel_case_types)]
                    struct SubmitJobSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::SubmitJobRequest>
                    for SubmitJobSvc<T> {
                        type Response = super::SubmitJobReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::SubmitJobRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).submit_job(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = SubmitJobSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                "/coordinator.Coordinator/PollJob" => {
                    #[allow(non_camel_case_types)]
                    struct PollJobSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::PollJobRequest>
                    for PollJobSvc<T> {
                        type Response = super::PollJobReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::PollJobRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).poll_job(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = PollJobSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                "/coordinator.Coordinator/Heartbeat" => {
                    #[allow(non_camel_case_types)]
                    struct HeartbeatSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::HeartbeatRequest>
                    for HeartbeatSvc<T> {
                        type Response = super::HeartbeatReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::HeartbeatRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).heartbeat(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = HeartbeatSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                "/coordinator.Coordinator/Register" => {
                    #[allow(non_camel_case_types)]
                    struct RegisterSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::RegisterRequest>
                    for RegisterSvc<T> {
                        type Response = super::RegisterReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::RegisterRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).register(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = RegisterSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                "/coordinator.Coordinator/GetTask" => {
                    #[allow(non_camel_case_types)]
                    struct GetTaskSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::GetTaskRequest>
                    for GetTaskSvc<T> {
                        type Response = super::GetTaskReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::GetTaskRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).get_task(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = GetTaskSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                "/coordinator.Coordinator/FinishTask" => {
                    #[allow(non_camel_case_types)]
                    struct FinishTaskSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::FinishTaskRequest>
                    for FinishTaskSvc<T> {
                        type Response = super::FinishTaskReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::FinishTaskRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).finish_task(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = FinishTaskSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                "/coordinator.Coordinator/FailTask" => {
                    #[allow(non_camel_case_types)]
                    struct FailTaskSvc<T: Coordinator>(pub Arc<T>);
                    impl<
                        T: Coordinator,
                    > tonic::server::UnaryService<super::FailTaskRequest>
                    for FailTaskSvc<T> {
                        type Response = super::FailTaskReply;
                        type Future = BoxFuture<
                            tonic::Response<Self::Response>,
                            tonic::Status,
                        >;
                        fn call(
                            &mut self,
                            request: tonic::Request<super::FailTaskRequest>,
                        ) -> Self::Future {
                            let inner = self.0.clone();
                            let fut = async move { (*inner).fail_task(request).await };
                            Box::pin(fut)
                        }
                    }
                    let accept_compression_encodings = self.accept_compression_encodings;
                    let send_compression_encodings = self.send_compression_encodings;
                    let inner = self.inner.clone();
                    let fut = async move {
                        let inner = inner.0;
                        let method = FailTaskSvc(inner);
                        let codec = tonic::codec::ProstCodec::default();
                        let mut grpc = tonic::server::Grpc::new(codec)
                            .apply_compression_config(
                                accept_compression_encodings,
                                send_compression_encodings,
                            );
                        let res = grpc.unary(method, req).await;
                        Ok(res)
                    };
                    Box::pin(fut)
                }
                _ => {
                    Box::pin(async move {
                        Ok(
                            http::Response::builder()
                                .status(200)
                                .header("grpc-status", "12")
                                .header("content-type", "application/grpc")
                                .body(empty_body())
                                .unwrap(),
                        )
                    })
                }
            }
        }
    }
    impl<T: Coordinator> Clone for CoordinatorServer<T> {
        fn clone(&self) -> Self {
            let inner = self.inner.clone();
            Self {
                inner,
                accept_compression_encodings: self.accept_compression_encodings,
                send_compression_encodings: self.send_compression_encodings,
            }
        }
    }
    impl<T: Coordinator> Clone for _Inner<T> {
        fn clone(&self) -> Self {
            Self(self.0.clone())
        }
    }
    impl<T: std::fmt::Debug> std::fmt::Debug for _Inner<T> {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            write!(f, "{:?}", self.0)
        }
    }
    impl<T: Coordinator> tonic::transport::NamedService for CoordinatorServer<T> {
        const NAME: &'static str = "coordinator.Coordinator";
    }
}
