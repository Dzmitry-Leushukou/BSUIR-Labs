from fastapi import Request
import logging

def get_client_ip(request: Request) -> str:
    """
    Получает IP-адрес клиента из запроса FastAPI
    Проверяет различные заголовки, которые могут содержать IP-адрес
    """
    # Проверяем заголовки в порядке приоритета
    forwarded_for = request.headers.get("x-forwarded-for")
    if forwarded_for:
        # X-Forwarded-For может содержать список IP-адресов, берем первый (реальный клиент)
        ip = forwarded_for.split(",")[0].strip()
        return ip
    
    real_ip = request.headers.get("x-real-ip")
    if real_ip:
        return real_ip.strip()
    
    forwarded_host = request.headers.get("x-forwarded-host")
    forwarded_proto = request.headers.get("x-forwarded-proto")
    forwarded_port = request.headers.get("x-forwarded-port")
    
    # Если есть другие заголовки, связанные с прокси
    for header_name in ["cf-connecting-ip", "true-client-ip", "x-client-ip", "x-cluster-client-ip"]:
        header_value = request.headers.get(header_name)
        if header_value:
            # Эти заголовки могут содержать один IP
            return header_value.strip()
    
    # Если нет заголовков прокси, используем клиентский IP из запроса
    if request.client and request.client.host:
        return request.client.host
    
    # В крайнем случае возвращаем localhost
    return "127.0.0.1"

def get_user_agent(request: Request) -> str:
    """
    Получает User-Agent из заголовков запроса
    """
    user_agent = request.headers.get("user-agent", "")
    return user_agent