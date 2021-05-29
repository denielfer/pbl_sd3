# -*- coding: utf-8 -*-

# This sample demonstrates handling intents from an Alexa skill using the Alexa Skills Kit SDK for Python.
# Please visit https://alexa.design/cookbook for additional examples on implementing slots, dialog management,
# session persistence, api calls, and more.
# This sample is built using the handler classes approach in skill builder.
import logging
import ask_sdk_core.utils as ask_utils

from ask_sdk_core.skill_builder import SkillBuilder
from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.dispatch_components import AbstractExceptionHandler
from ask_sdk_core.handler_input import HandlerInput

from ask_sdk_model import Response

import MQTT

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)




class LaunchRequestHandler(AbstractRequestHandler):
    """Handler for Skill Launch."""
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool

        return ask_utils.is_request_type("LaunchRequest")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        MQTT.iniciar()
        MQTT.conectar()
        MQTT.subs()
        speak_output = "Bem vindo, aqui você consegue informações sobre o dispositivo. Fale 'alexa me ajude' para mais informações"

        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(speak_output)
                .response
        )

#import time
#time.sleep(3)
class HelloWorldIntentHandler(AbstractRequestHandler):
    """Handler for Hello World Intent."""
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_intent_name("HelloWorldIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        speak_output = "Hello World!"

        return (
            handler_input.response_builder
                .speak(speak_output)
                # .ask("add a reprompt if you want to keep the session open for the user to respond")
                .response
        )


class HelpIntentHandler(AbstractRequestHandler):
    """Handler for Help Intent."""
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_intent_name("AMAZON.HelpIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        #Voce pode falar 'inverter estado de operação', para alterar o modo de operação do sensor. Ou 'alexa me informe sobre o estado do dispositivo' para obter informações sobre o estado do dispositvo. Ou 'definir tempo de espera para ' e dizer um numero para nos definirmos esse valor como tempo de espera, lembre que esse tempo sera em minutos e deve ser maior que 1. Para finalizar diga 'adeus'
        speak_output = "Olá, você pode inverter o estado de operação do sensor, requisitar informações sobre o estado do dispositivo, ou definir o tempo de espera do dispositivo. Lembre que esse tempo será em minutos, e deve ser maior que 1. Para finalizar diga 'adeus'"
        r2 = "Posso ajudar em mais alguma coisa?"
        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(r2)
                .response
        )


class CancelOrStopIntentHandler(AbstractRequestHandler):
    """Single handler for Cancel and Stop Intent."""
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return (ask_utils.is_intent_name("AMAZON.CancelIntent")(handler_input) or
                ask_utils.is_intent_name("AMAZON.StopIntent")(handler_input))

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        speak_output = "Até mais!"

        return (
            handler_input.response_builder
                .speak(speak_output)
                .response
        )

class FallbackIntentHandler(AbstractRequestHandler):
    """Single handler for Fallback Intent."""
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_intent_name("AMAZON.FallbackIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.info("In FallbackIntentHandler")
        speech = "Hmmm, acho que eu não entendi essa. Você pode inverter o estado de operação do sensor, requisitar informações sobre o estado do dispositivo, ou definir o tempo de espera do dispositivo. Lembre que esse tempo será em minutos, e deve ser maior que 1. Para finalizar diga 'adeus'"
        reprompt = "Desculpe eu não consegui entender, no que eu posso te ajudar?"

        return handler_input.response_builder.speak(speech).ask(reprompt).response

class SessionEndedRequestHandler(AbstractRequestHandler):
    """Handler for Session End."""
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_request_type("SessionEndedRequest")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response

        # Any cleanup logic goes here.

        return handler_input.response_builder.response


class IntentReflectorHandler(AbstractRequestHandler):
    """The intent reflector is used for interaction model testing and debugging.
    It will simply repeat the intent the user said. You can create custom handlers
    for your intents by defining them above, then also adding them to the request
    handler chain below.
    """
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_request_type("IntentRequest")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        intent_name = ask_utils.get_intent_name(handler_input)
        speak_output = "você ativou " + intent_name + "."

        return (
            handler_input.response_builder
                .speak(speak_output)
                # .ask("add a reprompt if you want to keep the session open for the user to respond")
                .response
        )


class CatchAllExceptionHandler(AbstractExceptionHandler):
    """Generic error handling to capture any syntax or routing errors. If you receive an error
    stating the request handler chain is not found, you have not implemented a handler for
    the intent being invoked or included it in the skill builder below.
    """
    def can_handle(self, handler_input, exception):
        # type: (HandlerInput, Exception) -> bool
        return True

    def handle(self, handler_input, exception):
        # type: (HandlerInput, Exception) -> Response
        logger.error(exception, exc_info=True)

        speak_output = "Desculpe, eu tive um problema fazendo o que você pediu, por favor tente novamente."

        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(speak_output)
                .response
        )
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# MINHAS CLASSES
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class set_time_IntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_intent_name("set_time_Intent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        tempo = handler_input.request_envelope.request.intent.slots['tempo'].value
        if(tempo == None):
            speak_output = f"Nao foi possível entender o numero informado, por favor tente de novo."
        else:
            tempo = int(tempo)
            if(tempo < 1):
                speak_output = f"O tempo informado deve ser maior que 1, e tem que ser um número inteiro. por favor tente novamente"
            else:
                speak_output = f"o tempo de tolerância de conexão foi definido para {tempo}"
                MQTT.publish("set_timer_alexa",{"timer":tempo*60})
        return ( handler_input.response_builder.speak(speak_output).ask(speak_output).response )


class get_status_IntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_intent_name("get_status_Intent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        MQTT.flag = True
        MQTT.publish("get_status_alexa",{})
        while(not(MQTT.flag)):
            pass
        speak_output = f'O estado do dispositivo é {MQTT.status}, e está no modo {MQTT.state}'
        r2 = "Deseja mais alguma coisa?"
        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(r2)
                .response
        )

class set_state_IntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        return ask_utils.is_intent_name("set_state_Intent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        MQTT.publish("set_state_alexa", {"estado": 0 if (MQTT.state == "Alarme") else 1})
        speak_output = f'O modo de operação do dispositivo foi alterado'
        r2 = "Deseja mais alguma coisa?"
        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(r2)
                .response
        )
# The SkillBuilder object acts as the entry point for your skill, routing all request and response
# payloads to the handlers above. Make sure any new handlers or interceptors you've
# defined are included below. The order matters - they're processed top to bottom.


sb = SkillBuilder()

sb.add_request_handler(LaunchRequestHandler())
sb.add_request_handler(HelloWorldIntentHandler())
sb.add_request_handler(HelpIntentHandler())
sb.add_request_handler(CancelOrStopIntentHandler())
sb.add_request_handler(FallbackIntentHandler())
sb.add_request_handler(SessionEndedRequestHandler())

sb.add_request_handler(set_time_IntentHandler())
sb.add_request_handler(get_status_IntentHandler())

sb.add_request_handler(set_state_IntentHandler())
sb.add_request_handler(IntentReflectorHandler()) # make sure IntentReflectorHandler is last so it doesn't override your custom intent handlers

sb.add_exception_handler(CatchAllExceptionHandler())

lambda_handler = sb.lambda_handler()